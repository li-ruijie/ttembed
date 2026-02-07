# ttembed

> This is a fork of "embed" by [Tom Murphy VII](http://carnage-melon.tom7.org/embed/), which was released into the public domain.

Remove embedding limitations from TrueType fonts by setting the `fsType` field in the OS/2 table to zero.

## Usage

```
ttembed [-n] font.ttf [font2.ttf ...]
```

- `-n` - Dry-run mode: report fonts with non-zero fsType without modifying them

## Example

```
$ find /usr/share/fonts -iname '*.?tf' -print0 | xargs -0 ttembed -n
fstype=0008 /usr/share/fonts/google-droid/DroidSans.ttf
fstype=0004 /usr/share/fonts/tlomt-junction/junction.otf
```

## Improvements over original

- Validates TTF/OTF format before modifying
- Correctly recalculates OS/2 table checksum (32-bit word-wise sum)
- `-n` dry-run option
- Batch processing of multiple fonts

## Disclaimer

Do not use this program to change embedding settings on fonts you did not create or are not licensed to modify.
