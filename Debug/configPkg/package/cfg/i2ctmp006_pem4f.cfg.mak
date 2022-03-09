# invoke SourceDir generated makefile for i2ctmp006.pem4f
i2ctmp006.pem4f: .libraries,i2ctmp006.pem4f
.libraries,i2ctmp006.pem4f: package/cfg/i2ctmp006_pem4f.xdl
	$(MAKE) -f /home/guest/ide/default/i2ctmp006_EK_TM4C123GXL_TI/src/makefile.libs

clean::
	$(MAKE) -f /home/guest/ide/default/i2ctmp006_EK_TM4C123GXL_TI/src/makefile.libs clean

