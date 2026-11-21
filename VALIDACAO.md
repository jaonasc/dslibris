# Validação — DSLibris Modern

## Revisão de estabilidade — 21/11/2026

Foram reproduzidas falhas da base anterior antes de corrigi-las, usando o código real de livros, páginas, EPUB, preferências e callbacks de XML em testes de host com AddressSanitizer, LeakSanitizer e UndefinedBehaviorSanitizer. Somente as partes de hardware e, nos testes de parser, a medição das fontes são substituídas.

Correções desta revisão:

- Liberação das páginas e dos buffers ao fechar um livro; recursos de EPUB e contextos de metadados agora têm duração limitada à operação.
- Erros de ZIP/XML preservados até o chamador; arquivos danificados ou sem páginas não entram no modo de leitura.
- Escritas no buffer de página e profundidade da pilha de tags verificadas. Conteúdo que excede os limites é rejeitado, em vez de sobrescrever memória. O buffer passou de 2 para 8 KiB.
- Posições negativas e marcadores antigos fora do intervalo ficam limitados às páginas carregadas.
- Fonte padrão usada quando a fonte salva foi removida; validação de substituições e liberação única de fontes compartilhadas.
- Preferências validadas antes de aplicar mudanças; fonte, margens, orientação e espaçamento importados ficam dentro dos limites da interface.
- Caracteres especiais de nomes de arquivos e pastas são escapados no XML.
- Gravação em `.tmp`, rotação de `.bak` e recuperação de backup. O backup recuperado é preservado mesmo se a promoção do novo arquivo falhar. Isso reduz o risco de perda, mas não torna uma interrupção de energia inofensiva em qualquer cartão.
- Falha ao abrir o log não derruba o leitor; erro ao abrir um livro aparece na interface.
- Caminho de XHTML corrigido para inserir o separador entre pasta e arquivo.

Resultados:

- Dez grupos de regressão passaram: ciclo de vida (2.400 páginas alocadas/liberadas), profundidade de tags, excesso de marcadores de estilo, EPUB inválido, EPUB vazio, 40 aberturas repetidas, XML com caracteres especiais, limites de preferências, recuperação de XML incompleto e falha do log.
- Teste com FreeType real passou: fonte salva ausente, fonte substituta inválida, troca válida e destruição dos recursos.
- Testes separados passaram para limites de posições/marcadores e falha de promoção de preferências após recuperação de backup.
- Cinco cenários em DeSmuME passaram: animação ligada, desligada, orientação invertida, biblioteca vazia e EPUB danificado seguido de livro válido. Foram conferidas as capturas de tela; os cenários normais incluem abertura/fechamento repetidos e comparações dos quadros de leitura.
- O executável final recebeu ainda uma nova rodada nativa após a última correção de backup. SHA-256: `978a0c8389dd6b3be19f7fd864044eeaf2424caaeeac5f30c6e742f8c27f5934`.

Uso pretendido informado pelo usuário: TWiLight Menu++. A instalação continua na raiz do cartão, junto de `Font` e `Book`; não foi realizado um ensaio do carregador no console físico.

Os scripts de regressão estão em `test/host-build.sh`, `test/run_stability.py`, `test/host-font-build.sh`, `test/run_font_test.py`, `test/book-bounds-build.sh`, `test/storage_test.cpp`, `test/emulator_matrix.py` e `test/final_native.py`. Os testes de host usam cabeçalhos de desenvolvimento de Expat/FreeType em `/tmp/dslibris-host/root/usr/include`; os de emulador são opcionais e requerem o núcleo preparado descrito abaixo.

Implementado em 20/11/2026 sobre a tag upstream `v1.5.2-2`, commit `1f6154863d3a6553dac66d5ca17875324da4e32c`.

## Resultado

- Compilação ARM9/ARM7 concluída com o SDK da imagem oficial `devkitpro/devkitarm:20240511`, extraído em uma pasta temporária. A ROM principal mantém o DLDI padrão para o carregador aplicar o driver apropriado.
- Testes C++ executados com `-Wall -Wextra -Werror -fsanitize=address,undefined`: contraste dos temas, extremos do alpha, UTF-8 válido/inválido/truncado, desenho recortado, canários antes/depois do buffer, preservação do preenchimento de cada linha e transições nas duas direções. Todos passaram.
- Executado o `.nds` em DeSmuME por sua API libretro, sem automação da interface do computador. O cartão virtual usa somente uma cópia descartável das fontes e do EPUB `quickstart.epub` fornecido com o programa. O DLDI MPCF foi aplicado somente à cópia usada nesse teste.
- Conferidas visualmente as telas reais da biblioteca, configurações, seleção de fontes e leitura nos quatro temas.
- O teste nativo compara os quadros completos: avançar altera a página; voltar restaura exatamente a primeira página; X altera as cores.
- Selecionada deliberadamente uma fonte inválida: o leitor apresentou a mensagem de erro e continuou abrindo o EPUB com a fonte anterior.
- Revisão independente encontrou o risco de descarte prematuro de fonte válida; a seleção agora valida a substituta antes de liberar os recursos ativos.
- Corrigidos vazamento dos bitmaps de letras, tamanho do buffer de desenho, corte de títulos e tratamento de biblioteca vazia. Preferências modernas são separadas das originais.

## Comandos principais

```sh
python3 scripts/fetch-sdk.py /tmp/dslibris-sdk
sh scripts/build.sh
sh scripts/test.sh
```

O emulador de teste foi compilado a partir de `libretro/desmume`, revisão `95b4d79`, com suporte a cartão virtual e renderização por software. Os scripts opcionais em `test/prepare_emulator.py`, `test/setup_smoke.py` e `test/emulator_smoke.py` documentam o ensaio. O emulador não é necessário para usar a ROM.

## Limites

Ainda sem ensaio em um DS/DSi físico. A duração e a suavidade efetivas da animação, o brilho e a integração com cada carregador/flashcart dependem desse ensaio. A meta de 12 quadros não representa uma medição de 60 fps no aparelho.

A virada é uma aproximação 2D com folha deslizando, borda e sombra. É possível desativá-la. Se os dois buffers temporários de 128 KiB não puderem ser alocados, o leitor desenha a página diretamente.

O suporte a EPUB e a paginação continuam derivados do programa original. O teste principal usa EPUB. Livros muito grandes e recursos de EPUB não suportados pelo leitor original continuam exigindo avaliação; não há garantia de abrir qualquer EPUB disponível.

As imagens em `previews/` são capturas nativas do emulador, giradas para a orientação de livro. Não são maquetes.

O original `dslibris.nds` foi preservado, SHA-256:
`16597f690fc99b4fe4768f943379dde4cd1262af0a1d17a9b0304945b2ca6757`.
