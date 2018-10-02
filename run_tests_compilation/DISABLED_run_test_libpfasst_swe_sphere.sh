#! /bin/bash

cd "$SWEET_ROOT"

SCONS="scons --program=libpfasst_swe_sphere --quadmath=enable  --libpfasst=enable --sweet-mpi=enable --libsph=enable --numa-block-allocator=0 --plane-spectral-space=disable --sphere-spectral-space=enable --threading=off --libfft=enable --sphere-spectral-dealiasing=enable"
echo "$SCONS"
$SCONS  || exit

