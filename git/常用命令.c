1. git init
	将当前的目录变成git可以管理的仓库
	
2. git add xxx.txt
	把xxx.txt添加到暂存区
	
3. git commit --m "提交的内容说明"
	将暂存区的所有内容提交到当前分支的本地仓库
	
   git commit -a -m "提交的内容说明"
		直接将工作区中的所有修改提交到当前分支的本地仓库
   git commit --amend
		这个命令也是进行提交的，不过此次提交会和上次提交的使用同一个commit-id，多用于在上次提交时漏提或者错提的情况
		
4. git status
	实时掌握仓库当前分支的状态，是否有修改
	
5. git diff
	此命令比较的是工作区中当前文件和暂存区域快照之间的差异
   
   git diff xxx.txt
	相比上面的命令，这个命令仅仅针对xxx.txt这个文件
	
   git diff HEAD -- xxx.txt
    查看工作区和版本库里面最新版本的区别
	
   git diff --staged
	这条命令将比对已暂存文件与最后一次提交的文件的差异
   
   git diff --cached
	查看已经暂存起来的的变化
	
6. git log
	查看修改历史

   git log --pretty=oneline
	查看修改历史，以单行显示
	
   git log --author="xxx"
    查看某个用户的所有提交
	
   git log --graph --pretty=oneline --abbrev-commit
    查看分支的合并情况
	
7. git reset --hard HEAD^
	版本回退，^表示上个版本，^^表示上上个版本，以此类推，注意本地的修改会丢失
	
   git reset --hard c3c2f33c89a6cf6006c4ac03609de0e4a73eb2ec
    版本回退到相应的commit-id对应的版本号，版本号可以不写全，前几位就可以
	
   git reset HEAD xxx.txt
    将暂存区退回到工作区
   
   git reset --soft HEAD^
    git 回退最近一次的提交并保留修改，该修改处于git add的状态

8. git reflog
	查看版本变更历史，跟git log的区别是，版本回退之后，git reflog会记录版本库中所有的变更，包括回退了的版本

9. git pull
	从远程库拉取所有分支跟本地分支合并
	
   git pull origin master
    把远程master分支和本地master分支合并
   
   git pull origin branch1:branch2
	把远程branch1分支和本地branch2分支合并
	
10. git checkout -- xxx.txt
	将xxx.txt文件在工作区中的修改全部撤销，有两种情况
	（1）没添加到暂存区，则返回最近一次git commit的状态
	（2）已经添加到暂存区，则返回最近一次git add的状态
	
	git checkout .
	相对于上面命令，这是针对所有文件的修改的
	
11. git branch
	查看当前分支
	
	git branch -a
	查看所有分支
	
	git branch dev
	创建dev分支
	
	git branch -d dev
	删除dev分支
	
	git branch -D dev
	强制删除还没合并的dev分支
	
12. git checkout dev
	切换到dev分支

    git checkout -b dev
	创建dev分支，-b参数表示创建并切换到该分支
	
13. git merge dev
	合并dev分支到当前分支

	git merge --no-ff -m "说明" dev
	准备合并dev分支
	--no-ff参数：表示禁用 “Fast forward”模式，通常，在合并分支时，如果可能，git会使用“Fast forward”模式，在这种模式下，删除分支后，会丢掉分支信
	息，如果要强制禁用“Fast forward”模式，git就会在merge时生成一个新的commit，这样，从分支历史上就可以看出分支信息
	
14. git remote
	查看远程库信息
	
	git remote -v
	查看远程库更详细信息，如果没有推送权限，就看不到push的地址
	$ git remote -v
	origin  git@github.com:coreJJ-tan/projectForSu.git (fetch)
	origin  git@github.com:coreJJ-tan/projectForSu.git (push)
	
15. git push
	将本地所有分支的提交提到远程库
	
	git push origin dev
	将本地dev分支提交到远程库

16. git show commit-id
	查看commit-id对应的版本的更改内容
	
	git show commit-id  xxx.txt
	查看commit-id对应的版本的xxx.txt文件的更改内容

17. git 添加空目录
刚建立项目时我们为了保证项目结构完整，要提交空的文件夹 ， 但是git默认是忽略空目录的。使用如下命令在工程路径下新建文件，再使用git add 添加工程。
    find . -type d -empty -exec touch {}/.gitignore \;

18. git如何撤销工作取得修改
对工作区中文件的修改分为三种情况：
（1）还没有git add
	直接使用 git checkout xxx.txt，即撤销修改，就回到和版本库一模一样的样子
（2）已经 git add
	先使用 git reset HEAD xxx.txt，然后再使用 git checkout -- xxx.txt进行修改撤销
（3）已经git add，并再次进行修改
	先使用 git checkout xxx.txt，文件就会变成添加到暂存区后的状态，也就是（2）的状态
总之，记住下面的两点：
	git checkout xxx.txt	撤销的是工作区中文件的修改
	git reset HEAD xxx.txt	撤销的是暂存区中文件的修改
	
19. 当 git pull 冲突，合并失败时
按照如下流程：
	git stash	备份当前工作取的内容
	git pull
	git stash pop	恢复工作区的内容
	此时不冲突的文件处于git add状态，可以使用 git reset HEAD xxx.txt 退回工作区
	对于冲突的文件，使用git status会提示both modified，手动修改后便处于git add的状态
	
	git stash	显示git栈中的所有备份
	git stash clear		清空git 栈
	
20. git commit --amend 之后的撤销方法
	在提patch的时候有时候会误操作，在git add之后，提交时不小心执行为git commit --amend，这时执行git reflog，然后git reset + 想恢复到的地方的id，
就可以恢复到任一个被覆盖的id了，且对文件的修改回退到代码仓库not staged的状态了，不使用git reset --hard 的目的就是保留本地修改，否则修改就会被丢
弃，切记慎用--hard参数，除非你确定放弃当前未提交的所有修改！
	总结如上步骤：
	git reflog	查看操作记录
	git reset --soft HEAD@{1}	当前分支的HEAD指向HEAD@{1}，即可达到撤销 amend 的目的
	git reset HEAD xxx.txt		即可将文件退回工作区