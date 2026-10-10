#include <stddef.h>
#include <string.h>
#include <vdso.h>

// минимальные ELF64-типы (freestanding, без elf.h)
typedef struct {
    unsigned char e_ident[16];
    unsigned short e_type, e_machine;
    unsigned e_version;
    unsigned long e_entry, e_phoff, e_shoff;
    unsigned e_flags;
    unsigned short e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum,
        e_shstrndx;
} Ehdr;

typedef struct {
    unsigned p_type, p_flags;
    unsigned long p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align;
} Phdr;

typedef struct {
    unsigned long d_tag;
    unsigned long d_val;
} Dyn;

typedef struct {
    unsigned st_name;
    unsigned char st_info, st_other;
    unsigned short st_shndx;
    unsigned long st_value, st_size;
} Sym;

#define PT_LOAD 1
#define PT_DYNAMIC 2
#define DT_NULL 0
#define DT_HASH 4
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_GNU_HASH 0x6ffffef5

vdso_clock_gettime_fn __vdso_clock_gettime_f;
vdso_clock_getres_fn __vdso_clock_getres_f;
vdso_gettimeofday_fn __vdso_gettimeofday_f;
vdso_time_fn __vdso_time_f;
int __vdso_active;

struct gnu_hash {
    unsigned nbucket, symoffset, bloom_size, bloom_shift;
};

struct sysv_hash {
    unsigned nbucket, nchain;
};

static unsigned gnu_hash(const char *s)
{
    unsigned h = 5381;
    while (*s)
        h = h * 33 + (unsigned char)*s++;
    return h;
}

static unsigned sysv_hash(const char *s)
{
    unsigned h = 0, g;
    while (*s) {
        h = (h << 4) + (unsigned char)*s++;
        g = h & 0xf0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

// все адреса внутри vDSO — link-time vaddr, приводятся через bias
static void *resolve_gnu(size_t bias, const Sym *symtab, const char *strtab,
                         const struct gnu_hash *gh, const char *name)
{
    unsigned h = gnu_hash(name);
    const unsigned *buckets =
        (const unsigned *)((const char *)gh + 16 +
                           (size_t)gh->bloom_size * sizeof(unsigned long));
    const unsigned *chain = buckets + gh->nbucket;

    unsigned i = buckets[h % gh->nbucket];
    if (i < gh->symoffset)
        return 0;
    for (;; i++) {
        unsigned hv = chain[i - gh->symoffset];
        // LSB слова цепочки — признак конца, из сравнения он стирается
        if ((hv | 1) == (h | 1)) {
            const Sym *sym = &symtab[i];
            if (sym->st_value && !strcmp(strtab + sym->st_name, name))
                return (void *)(bias + sym->st_value);
        }
        if (hv & 1)
            break;
    }
    return 0;
}

static void *resolve_sysv(size_t bias, const Sym *symtab, const char *strtab,
                          const struct sysv_hash *sh, const char *name)
{
    unsigned h = sysv_hash(name);
    const unsigned *buckets = (const unsigned *)((const char *)sh + 8);
    const unsigned *chain = buckets + sh->nbucket;

    for (unsigned i = buckets[h % sh->nbucket]; i; i = chain[i]) {
        const Sym *sym = &symtab[i];
        if (sym->st_value && !strcmp(strtab + sym->st_name, name))
            return (void *)(bias + sym->st_value);
    }
    return 0;
}

static void *findsym(size_t bias, const Sym *symtab, const char *strtab,
                     const struct gnu_hash *gh, const struct sysv_hash *sh,
                     const char *name)
{
    char sym[64];
    strcpy(sym, "__vdso_");
    strcat(sym, name);
    void *r = resolve_gnu(bias, symtab, strtab, gh, sym);
    if (!r)
        r = resolve_sysv(bias, symtab, strtab, sh, sym);
    if (!r)
        r = resolve_gnu(bias, symtab, strtab, gh, name);
    if (!r)
        r = resolve_sysv(bias, symtab, strtab, sh, name);
    return r;
}

void __init_vdso(void *base_ptr)
{
    size_t eh = (size_t)base_ptr;
    if (!eh)
        return;

    if (memcmp((const void *)eh, "\x7f" "ELF", 4) != 0)
        return;

    Ehdr *hdr = base_ptr;
    Phdr *ph = (Phdr *)(eh + hdr->e_phoff);
    size_t bias = 0, dyn_off = 0;
    int load_found = 0;
    for (int i = 0; i < hdr->e_phnum;
         i++, ph = (Phdr *)((char *)ph + hdr->e_phentsize)) {
        if (ph->p_type == PT_LOAD) {
            bias = eh + ph->p_offset - ph->p_vaddr;
            load_found = 1;
        } else if (ph->p_type == PT_DYNAMIC) {
            dyn_off = ph->p_offset;
        }
    }
    if (!load_found || !dyn_off)
        return;
    Dyn *d = (Dyn *)(eh + dyn_off);

    size_t symtab = 0, strtab = 0, gh = 0, sh = 0;
    for (; d->d_tag != DT_NULL; d++) {
        switch (d->d_tag) {
        case DT_STRTAB:
            strtab = bias + d->d_val;
            break;
        case DT_SYMTAB:
            symtab = bias + d->d_val;
            break;
        case DT_GNU_HASH:
            gh = bias + d->d_val;
            break;
        case DT_HASH:
            sh = bias + d->d_val;
            break;
        }
    }
    if (!symtab || !strtab || (!gh && !sh))
        return;

    __vdso_clock_gettime_f =
        findsym(bias, (const Sym *)symtab, (const char *)strtab,
                (const struct gnu_hash *)gh, (const struct sysv_hash *)sh,
                "clock_gettime");
    __vdso_clock_getres_f =
        findsym(bias, (const Sym *)symtab, (const char *)strtab,
                (const struct gnu_hash *)gh, (const struct sysv_hash *)sh,
                "clock_getres");
    __vdso_gettimeofday_f =
        findsym(bias, (const Sym *)symtab, (const char *)strtab,
                (const struct gnu_hash *)gh, (const struct sysv_hash *)sh,
                "gettimeofday");
    __vdso_time_f = findsym(bias, (const Sym *)symtab, (const char *)strtab,
                            (const struct gnu_hash *)gh,
                            (const struct sysv_hash *)sh, "time");
    __vdso_active = __vdso_clock_gettime_f != 0;
}
