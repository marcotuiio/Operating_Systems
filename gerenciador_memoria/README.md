# Gerenciador de Memória

### Marco Túlio Alves de Barros     27/10/2023

## Introdução

O projeto tem como intenção simular um gerenciador de memória, usando linguagem C.

Deve fazer uso de uma cache TBL e uma tabela de páginas para a lógica de tradução e busca de endereços físicos.

O disco, ou seja, memória secundária, é simulado por um arquivo .bin definido no arquivo "libs.h".

## Lógica de funcionamento

Deve também converter os endereços lógicos, lidos de um arquivo .txt na entrada, em endereços físicos. Para tal os enedereços
lógicos são convertidos em binário e separados em 3 partes: offset (8 bits menos significativos), número da página (8 bits do meio) 
e endereço (16 bits mais significativos). 
A primeira busca de página é feita na cache TBL, caso não seja encontrada, é feita na tabela de páginas. Caso não seja encontrada
na tabela de páginas, é feita uma busca no disco, e a página é carregada na tabela de páginas e na cache TBL.

Para a busca no .bin, deve-se ter o número de página em questão, e trazer para a memória principal, 256 bytes a partir do endereço
da página em questão. Mas para tal a lógica é simples, sabendo que a memória tem 64kb, e que as páginas tem 256 bytes (então 256 endereços),
basta multiplicar o número da página por 256 e teremos o endereço de início da página em questão (o fim é o início + 256). 
O endereço físico é composto pelo endereço de início da página + o offset.

#### Estruturas usadas

* Memory:
    Armazena informações sobre a memória virtual, incluindo TLB, tabela de páginas e memória física.

* Entry:
    Representa uma entrada na TLB ou na tabela de páginas com informações de número de página e quadro.

* Frame:
    Representa um quadro na memória física com informações sobre sua disponibilidade e dados.

* Address:
    Contém informações sobre um endereço de memória, incluindo representação binária e campos separados.  
