### Comandos Essenciais para Limpeza do Codespace

Aqui estão os comandos mais úteis para liberar espaço de armazenamento no Codespace, organizados por categoria.

-----

### Mais Simples e Rápido

Limpa o cache de construção (Build Cache) do Docker
```bash
docker builder prune
```

### 1\. Limpeza Geral de Caches

Esses comandos são seguros para rodar e visam liberar espaço de caches de sistemas e gerenciadores de pacotes.

#### Para sistemas baseados em Debian/Ubuntu (`apt-get`)

```bash
sudo apt-get clean
```

  * Limpa o cache de pacotes baixados pelo `apt-get`.

#### Para gerenciadores de pacotes Node.js

```bash
# Para NPM
npm cache clean --force

# Para Yarn
yarn cache clean
```

  * Limpa os caches de pacotes Node.js, que podem ser grandes.

#### Para o gerenciador de pacotes Python (`pip`)

```bash
pip cache purge
```

  * Remove o cache de pacotes instalados pelo `pip`.

#### Para caches de compiladores e ferramentas

```bash
# Limpa o cache da extensão C/C++ do VS Code (vscode-cpptools)
# Este é frequentemente um dos maiores caches.
rm -rf ~/.cache/vscode-cpptools/
```

-----

### 2\. Encontrar e Remover Pastas Grandes

Se a limpeza de cache não for suficiente, você pode identificar e remover manualmente as pastas que mais consomem espaço.

#### Encontrar as 10 maiores pastas na pasta atual

```bash
du -sh * | sort -rh | head -10
```

  * Este comando lista as 10 maiores pastas (e arquivos) no diretório atual, ordenadas de forma decrescente. É a sua melhor ferramenta para identificar o que está ocupando espaço.

#### Excluir uma pasta (e todo o seu conteúdo)

```bash
rm -rf nome_da_da_pasta
```

  * **Atenção:** Use este comando com cautela. Certifique-se de que a pasta que você está removendo não é crítica para o seu sistema ou projeto. Pastas como `node_modules` ou `build` são seguras de remover, pois podem ser recriadas.

-----

### Dica Adicional: Automatize a Limpeza

Para evitar que seu Codespace lote novamente, considere adicionar comandos de limpeza ao seu script de inicialização ou ao `postCreateCommand` no seu arquivo `.devcontainer.json`.



### Garbage Collector do GIT
```bash
git gc --prune=now
```