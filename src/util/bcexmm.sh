#!/bin/bash
if [ -f "BCEXMM.MAP" ]; then
	mv BCEXMM.MAP bcexmm.meh
fi
	if [ -f "BCEXMM.EXE" ]; then
		mv BCEXMM.EXE bcexmm.ex0
		mv bcexmm.ex0 bcexmm.exe
	fi
if [ -f "BCBAKAPI.MAP" ]; then
	mv BCBAKAPI.MAP bcbakapi.meh
fi
	if [ -f "BCBAKAPI.EXE" ]; then
		mv BCBAKAPI.EXE bcbakapi.ex0
		mv bcbakapi.ex0 bcbakapi.exe
	fi

if [ -f "BCXMS.EXE" ]; then
    mv BCXMS.EXE bcxms.ex0
    mv bcxms.ex0 bcxms.exe
fi

if [ -f "BCINPU.EXE" ]; then
    mv BCINPU.EXE bcinpu.ex0
    mv bcinpu.ex0 bcinpu.exe
fi
