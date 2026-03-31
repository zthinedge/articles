 git init 
 ssh-keygen -t ed25519 -C "你的邮箱"
 cat ~/.ssh/id_ed25519.pub
 git remote add origin git@github.com:zymJDFH/articles.git
 git branch -M main
 git pull origin main
 git push -u origin main
