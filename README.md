Vitis Example
=============

Getting Started
---------------

1.  [Install Vitis][vitis]

2.  [Install Xilinx Run Time (XRT)][xrt]

3.  [Install PetaLinux][petalinux]

4.  Source the setup files for Vitis, XRT, and PetaLinux
    
        source /opt/Xilinx/Vitis/2019.2/settings64.sh
        source /opt/Xilinx/petalinux/2019.2/settings.sh
        source /opt/xilinx/xrt/setup.sh
    
    The paths may be different on your machine. Adjust accordingly.

5.  Run `make run` to build project and start software emulation

6.  Once the QEMU VM starts up, login with username `root` and password `root`,
    then run the following commands to launch software emulation:
    
        mount /dev/mmcblk0p1 /mnt
        cd /mnt
        ./init.sh
    
    Use `reboot` to shutdown the VM.

[vitis]: https://www.xilinx.com/html_docs/xilinx2019_2/vitis_doc/Chunk1858803630.html#ariaid-title2
[xrt]: https://www.xilinx.com/html_docs/xilinx2019_2/vitis_doc/Chunk1858803630.html#ariaid-title3
[petalinux]: https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/embedded-design-tools.html
