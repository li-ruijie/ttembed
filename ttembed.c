/*
 * ttembed - set TTF/OTF files to Installable Embedding mode.
 *
 * Copyright (c) 2013      Tom Murphy VII (original, public domain)
 * Copyright (c) 2026      Li Ruijie
 *
 * Note that using this to embed fonts which you are not licensed to embed
 * does not make it legal.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int readbe32(FILE *f, uint32_t *out)
{
    int b0, b1, b2, b3;
    if ((b0 = fgetc(f)) == EOF) return -1;
    if ((b1 = fgetc(f)) == EOF) return -1;
    if ((b2 = fgetc(f)) == EOF) return -1;
    if ((b3 = fgetc(f)) == EOF) return -1;
    *out = (uint32_t)b0 << 24 | (uint32_t)b1 << 16
         | (uint32_t)b2 << 8  | (uint32_t)b3;
    return 0;
}

int readbe16(FILE *f, uint16_t *out)
{
    int b0, b1;
    if ((b0 = fgetc(f)) == EOF) return -1;
    if ((b1 = fgetc(f)) == EOF) return -1;
    *out = (uint16_t)b0 << 8 | (uint16_t)b1;
    return 0;
}

void fstype0(char *filename, int dry_run)
{
    FILE *inways;
    if ((inways = fopen(filename, dry_run ? "rb" : "rb+"))) {
        int x, c;
        char type[5];
        type[4]=0;

        uint32_t ftype;
        uint16_t numTables;
        if (readbe32(inways, &ftype)) goto funk;
        if (ftype != 0x00010000 && ftype != 0x4f54544f) {
            fprintf(stderr, "%s: Not TTF/OTF\n", filename);
            fclose(inways);
            return;
        }
        if (readbe16(inways, &numTables)) goto funk;
        if (fseek(inways, 12, SEEK_SET)) goto funk;
        for (; numTables > 0; numTables--) {
            for (x=0;x<4;x++) {
                if ((c = getc(inways)) == EOF) goto funk;
                type[x] = c;
            }
            if (!strcmp(type,"OS/2")) {
                uint32_t checksum, offset, length;
                uint16_t oldfstype;
                uint32_t sum=0;
                long loc = ftell(inways); /* location for checksum */
                if (readbe32(inways, &checksum)) goto funk;
                if (readbe32(inways, &offset)) goto funk;
                if (readbe32(inways, &length)) goto funk;
                if (fseek(inways, offset+8, SEEK_SET)) goto funk;
                if (readbe16(inways, &oldfstype)) goto funk;
                if (oldfstype != 0) {
                    if (!dry_run) {
                        if (fseek(inways, offset+8, SEEK_SET)) goto funk;
                        if (fputc(0,inways) == EOF) goto funk;
                        if (fputc(0,inways) == EOF) goto funk;
                        if (fseek(inways, offset, SEEK_SET)) goto funk;
                        for (x = (length+3) & ~3; x > 0; x -= 4) {
                            uint32_t word;
                            if (readbe32(inways, &word)) goto funk;
                            sum += word;
                        }
                        if (fseek(inways, loc, SEEK_SET)) goto funk;
                        if (fputc(0xff & (sum>>24), inways) == EOF) goto funk;
                        if (fputc(0xff & (sum>>16), inways) == EOF) goto funk;
                        if (fputc(0xff & (sum>>8),  inways) == EOF) goto funk;
                        if (fputc(0xff & sum,       inways) == EOF) goto funk;
                    } else {
                        printf("fstype=%04x %s\n", oldfstype, filename);
                    }
                }
                fclose(inways);
                break;
            }
            for (x=12;x--;) if (EOF == getc(inways)) goto funk;
        }
        fprintf(stderr, "%s: No OS/2 table\n", filename);
        fclose(inways);
    } else {
        perror(filename);
    }
    return;
funk:
    fprintf(stderr, "%s: Malformed TTF\n", filename);
    fclose(inways);
    return;
}

int main (int argc, char *argv[])
{
    int dry_run = 0, i;

    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (argv[i][1] == 'n' && argv[i][2] == '\0')
            dry_run = 1;
        else {
            fprintf(stderr, "Usage: %s [-n] font.ttf ...\n", argv[0]);
            return 1;
        }
    }

    if (i >= argc) {
        fprintf(stderr,
            "Usage: %s [-n] font.ttf ...\n"
            "\n"
            "Remove embedding restrictions from TrueType/OpenType fonts\n"
            "by setting the fsType field in the OS/2 table to zero.\n"
            "\n"
            "Options:\n"
            "  -n    dry run: report non-zero fsType without modifying\n",
            argv[0]);
        return 1;
    }

    for (; i < argc; i++)
        fstype0(argv[i], dry_run);

    return 0;
}
