set(NVME_SEARCH_PATHS
    /usr/local
)

find_path(NVME_INCLUDE_DIR libnvme.h
    PATH_SUFFIXES include/
