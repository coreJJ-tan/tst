1、git 添加空目录
    刚建立项目时我们为了保证项目结构完整，要提交空的文件夹 ， 但是git默认是忽略空目录的。使用如下命令在工程路径下新建文件，再使用git add 添加工程。
    find . -type d -empty -exec touch {}/.gitignore \;

