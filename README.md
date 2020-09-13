### hugeelfexec

```
./hugeelfexec
Usage: ./hugeelfexec /path/to/bin args-for-bin
```

#### uses

- `memfd_create` (w/o `MFD_HUGETLB`, since it requires hugetlbfs setup)
- `madvise(MADV_HUGEPAGE)`
- `fexecat`

#### requirements

One of the following:

- `echo advise > /sys/kernel/mm/transparent_hugepage/shmem_enabled`
- `echo always > /sys/kernel/mm/transparent_hugepage/shmem_enabled`
- `echo within_size > /sys/kernel/mm/transparent_hugepage/shmem_enabled`

#### reference

- https://github.com/abbat/elfexec
