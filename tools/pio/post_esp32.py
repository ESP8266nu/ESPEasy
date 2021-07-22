Import("env")

def esp32_create_factory_bin(source, target, env):
    print("Generating factory bin for genuine esp units")
    #offset = 0x1000
    offset = 0x0
    new_file_name = env.subst("$BUILD_DIR/${PROGNAME}-factory.bin")
    sections = env.subst(env.get('FLASH_EXTRA_IMAGES'))
    new_file = open(new_file_name,"wb")
    for section in sections:
      sect_adr,sect_file = section.split(" ",1)
      source = open(sect_file,"rb")
      new_file.seek(int(sect_adr,0)-offset)
      new_file.write(source.read());
      source.close()
    
    firmware = open(env.subst("$BUILD_DIR/${PROGNAME}.bin"),"rb")
    new_file.seek(0x10000-offset)
    new_file.write(firmware.read())
    new_file.close()
    firmware.close()

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp32_create_factory_bin)
