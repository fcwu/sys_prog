#!/bin/bash

cleanup()
{
    umount ${MOUNT_POINT} || true
    rmdir ${MOUNT_POINT} || true
}

usage()
{
    echo "Usage: $0 iso_dir path_to_grub_cfg"
    echo "  If root in grub.cfg is not (hd0,msdos2), you can specify it by environment"
    echo "  variable ROOT_DEV=\"(hd?,msdos?)\""
    exit
}

die()
{
    echo "$1"
    exit
}

create_grub_cfg_template()
{
    echo "Creating grub.cfg template"
    cat <<EOF > $GRUB_CFG 
### BEGIN /etc/grub.d/00_header ###
if [ -s $prefix/grubenv ]; then
  set have_grubenv=true
  load_env
fi
set default="0"
if [ "${prev_saved_entry}" ]; then
  set saved_entry="${prev_saved_entry}"
  save_env saved_entry
  set prev_saved_entry=
  save_env prev_saved_entry
  set boot_once=true
fi

function savedefault {
  if [ -z "${boot_once}" ]; then
    saved_entry="${chosen}"
    save_env saved_entry
  fi
}

function recordfail {
  set recordfail=1
  if [ -n "${have_grubenv}" ]; then if [ -z "${boot_once}" ]; then save_env recordfail; fi; fi
}

function load_video {
  insmod vbe
  insmod vga
  insmod video_bochs
  insmod video_cirrus
}

insmod part_msdos
insmod ext2
set root='(hd0,msdos3)'
search --no-floppy --fs-uuid --set=root dcf73387-12ed-4e50-9803-3d2d9d2c3094
if loadfont /usr/share/grub/unicode.pf2 ; then
  set gfxmode=auto
  load_video
  insmod gfxterm
  insmod part_msdos
  insmod ext2
  set root='(hd0,msdos3)'
  search --no-floppy --fs-uuid --set=root dcf73387-12ed-4e50-9803-3d2d9d2c3094
  set locale_dir=($root)/boot/grub/locale
  set lang=en_US
  insmod gettext
fi
terminal_output gfxterm
set timeout=-1
### END /etc/grub.d/00_header ###

### BEGIN /etc/grub.d/05_debian_theme ###
set menu_color_normal=white/black
set menu_color_highlight=black/light-gray
if background_color 44,0,30; then
  clear
fi
### END /etc/grub.d/05_debian_theme ###
EOF
}

create_grub_cfg_boot_entry_for_iso()
{
    local iso=$1
    local iso_name=$(basename $iso)
    local vmlinuz=
    local initrd=
    if ! mount -t iso9660 -o ro $iso $MOUNT_POINT; then
        RET_STR="failed to mount $iso to $MOUNT_POINT" 
        return 1
    fi
    vmlinuz=$(cd $MOUNT_POINT && ls -1t install/vmlinu* casper/vmlinu* vmlinu* | head -1)
    initrd=$(cd $MOUNT_POINT && ls -1t install/initrd* casper/initrd* initrd* | head -1)
    if [ ! -f "${MOUNT_POINT}/$vmlinuz" ]; then
        RET_STR="No vmlinuz found: ${MOUNT_POINT}/$vmlinuz"
        umount $MOUNT_POINT
        return 2
    fi
    if [ ! -f "${MOUNT_POINT}/$initrd" ]; then
        RET_STR="No initrd found: ${MOUNT_POINT}/$initrd"
        umount $MOUNT_POINT
        return 3
    fi
    cat <<EOF >>$GRUB_CFG
menuentry "$iso_name" {
    set root=$ROOT_DEV
    set isofile="/$iso_name"
    loopback loop \$isofile
    linux (loop)/$vmlinuz boot=casper iso-scan/filename=\$isofile noprompt automatic-ubiquity quiet splash --
    initrd (loop)/$initrd
}
EOF
    umount $MOUNT_POINT
    return 0
}

[ $# -ne 2 -o "$1" == "-h" -o "$1" == "--help" ] && usage

exec 2>/dev/null
trap cleanup INT TERM
ISO_PATH=$1
GRUB_CFG=$2
[ -z "$ROOT_DEV" ] && ROOT_DEV="(hd0,msdos2)"
MOUNT_POINT=$(mktemp -d)

# arguments sentry
[ "$USER" != "root" ] && die "must run with root"
[ ! -d "$ISO_PATH" ] && die "$ISO_PATH is not directory"
[ -d "$GRUB_CFG" ] && GRUB_CFG="${GRUB_CFG}/grub.cfg"

rm -f $GRUB_CFG || die "failed to remove ${GRUB_CFG}"
create_grub_cfg_template || die "${GRUB_CFG} is not writable"

echo "Scanning $ISO_PATH"
for iso in $(ls $ISO_PATH/*.iso $ISO_PATH/iso/*.iso) ; do
    echo -n "    creating boot entry for file $iso..."
    if create_grub_cfg_boot_entry_for_iso "$iso" ; then
        echo "ok"
    else
        echo "fail: $RET_STR"
    fi
done

cleanup
