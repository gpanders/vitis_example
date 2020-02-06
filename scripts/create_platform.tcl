if { $::argc != 3 } {
    puts "Usage: $::argv0 <xsa file> <workspace> <build dir>"
    exit 1
}

set hw [file normalize [lindex $::argv 0]]
set name [file rootname [file tail $hw]]
set workspace [file normalize [lindex $::argv 1]]
set build_dir [file normalize [lindex $::argv 2]]

platform create -name $name -hw $hw -proc psu_cortexa53 -os linux -no-boot-bsp -prebuilt -out $workspace
domain config -image $build_dir/boot
domain config -sysroot $build_dir/sysroot
domain config -boot $build_dir/boot
domain config -bif pfm/linux.bif
domain config -qemu-args pfm/qemu/qemu_args.txt
domain config -pmuqemu-args pfm/qemu/pmu_args.txt
domain config -qemu-data $build_dir/boot
platform generate

file delete -force $build_dir/platform
file copy $workspace/$name/export/$name $build_dir/platform/
