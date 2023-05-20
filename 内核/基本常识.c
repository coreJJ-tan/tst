1、linux 源码官方网站：https://www.kernel.org/
2、在kernel的官网上有三种版本，mainline，stable，longterm
    mainline是主线版本，最新的   
    stable是稳定版
    longterm是长期支持版
    eol，当然就是不再支持

3、 Linux下面去编译项目之前，一般常会用make mrproper去先删除之前编译所生成的文件和配置文件，备份文件等，其中，mrproper和distclean，clean之间的区别，Linux内核源码根目
录下面的makefile中，有很清晰的解释：
help:
 @echo  'Cleaning targets:'
 @echo  '  clean    - Remove most generated files but keep the config and'
 @echo  '                    enough build support to build external modules'
 @echo  '  mrproper   - Remove all generated files + config + various backup files'
 @echo  '  distclean   - mrproper + remove editor backup and patch files'
