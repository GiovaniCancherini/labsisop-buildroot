💡 Dica: Configure a variável em Bash e Zsh (Buildroot)

Adicione a variável LINUX_OVERRIDE_SRCDIR aos dois arquivos de inicialização:
```
echo 'export LINUX_OVERRIDE_SRCDIR=/workspaces/labsisop-buildroot/labsysop-linux-4.13' | tee -a ~/.bashrc ~/.zshrc
```


Recarregue as configurações sem reiniciar o terminal:
```
source ~/.bashrc
source ~/.zshrc
```

Verifique se funcionou:
```
echo "$SHELL"
echo "$LINUX_OVERRIDE_SRCDIR"
```