# Brief Steps

## format and partition

```bash
$ dd if=/dev/zero of=vmhd bs=1M count=2
$ echo "n
p
1

3900
n
2
p


w
" | fdisk -c=dos vmhd
$ sudo losetup /dev/loop0 vmhd
$ sudo losetup -o $((512 * 32)) --sizelimit $(((3900 - 62) * 512)) /dev/loop0 vmhd
$ sudo losetup -o $((3901 * 32)) /dev/loop1 vmhd
$ mkdir mnt mnt2
$ sudo mkfs.ext2 -N 64 mnt -m 0 /dev/loop1
$ sudo mkfs.ext2 -N 64 mnt -m 0 /dev/loop2
$ sudo mount /dev/loop1 mnt
$ sudo mount /dev/loop2 mnt2
```

## grub

```
$ cd /path/to/grub/code && ./configure && make && sudo make install
$ mkdir -p test/boot
$ # small modify on grub-install in order to install correct device map
$ #     Use when boot-directory is different install device
$ sed -i 's#^grub_device=.*#grub_device=/dev/loop1#' /usr/local/sbin/grub-install
$ sudo mkdir -p mnt/boot/grub
$ sudo bash -c "cat > mnt/boot/grub/device.map" <<EOF
(hd0) /dev/loop0
(hd0,1) /dev/loop1
(hd0,2) /dev/loop2
EOF
$ sudo bash -c "cat > mnt/boot/grub/grub.cfg" <<EOF
set timeout=0
set default=0
set hidden
menuentry "My Linux kernel" {
    set root=(hd0,1)
    linux /boot/vmlinuz
    initrd /boot/initramfs.img
}
EOF
$ sudo /usr/local/sbin/grub-install --no-floppy --boot-directory=test/boot /dev/loop0
$ sudo cp test/boot/grub/{} mnt/boot/grub/
```

## kernel

```
$ cd /path/to/kernel
$ make menuconfig  # deselect all except PCI/PCIE/SATA
$ cp arch/x86/boot/bzImage /path/to/initramfs/boot/
```

## initramfs

```
$ mkdir initramfs
```

### init

```
$ cat > initramfs/init <<EOF
#!/bin/sh
init=/sbin/init

mount -t sysfs sysfs /sys
/sbin/mdev -s

exec $init

echo "Impossible here"
exec sh
```

### busybox

```
$ cd /path/to/busybox
$ make menuconfig
$ make install
$ cp -r _install/* /path/to/initramfs
$ # make some busybox downsize, then override busybox. I will talk this latter.
```

### /etc

passwd, group and fstab inittab


# Downsize

## Kernel

deselect all options except PCI/PCIE/SATA.

## initramfs

1. deselect most of busybox options
2. link(gcc) library for busybox as static one
3. relink it with uClibc libm.a and libcrypt.a

## Disk

1. Set rootfs as ramdisk
2. mount the partitions which will change after booting, such as /home.
3. Specify inode number and donot reserve space for super user.

# disk partition

Describe in [brief steps]

# login

Add following line in /etc/inittab
```
tty1::respawn:/sbin/getty 38400 tty1
```

# welcome message

Add a file called /etc/motd. The file will be print after loging by getty in busybox

