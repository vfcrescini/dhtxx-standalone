#!/bin/sh

# Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

# 2022-01-22 gentar.sh

# Generates tarball from the given filelist

TMPDIR="/tmp/.gentar.${$}"

if [ ${#} -lt 2 ]; then
  echo "Usage: gentar.sh <filelist> <tarball>" >&2
  exit 1
fi

path_flist=${1}
path_tball=${2}

mkdir -p ${TMPDIR}

cat ${path_flist} | sed -e 's/^\s*//' -e 's/#.*//g' -e '/^\s*$/d' -e 's/\s*:\s*/:/g' | while IFS=":" read f1 f2 f3 f4 f5 f6; do
  if [ "${f1}" = "F" ] || [ "${f1}" = "D" ]; then
      echo "${f2}" | grep -q '[0-7][0-7][0-7][0-7]'
      if [ ${?} -ne 0 ]; then
        echo "Invalid mode: ${f2}" >&2
        exit 2
      fi
      getent passwd "${f3}" > /dev/null 2>&1
      if [ ${?} -ne 0 ]; then
        echo "Invalid UID: ${f3}" >&2
        exit 3
      fi
      getent group "${f4}" > /dev/null 2>&1
      if [ ${?} -ne 0 ]; then
        echo "Invalid GID: ${f4}" >&2
        exit 4
      fi
  fi
  case ${f1} in
    F)
      if [ `echo "${f5}" | cut -c 1` = "/" ]; then
        if [ ! -f ${f5} ] || [ ! -r ${f5} ]; then
          echo "Not a file or not readable: ${f5}" >&2
          exit 5
        fi
        src=${f5}
      else
        if [ ! -f ${f5} ] || [ ! -r ${f5} ]; then
          echo "Not a file or not readable: ${f5}" >&2
          exit 6
        fi
        src=${f5}
      fi
      install -m ${f2} -o ${f3} -g ${f4} -T ${src} ${TMPDIR}/${f6}
      ;;
    D)
      install -m ${f2} -o ${f3} -g ${f4} -d ${TMPDIR}/${f5}
      ;;
    S)
      ln -fs ${f2} ${TMPDIR}/${f3}
      ;;
    *)
      echo "Unknown type: ${f1}" >&2
      exit 7
      ;;
  esac
done

tar -C ${TMPDIR} -czf ${path_tball} .
rm -fr ${TMPDIR}

exit 0
