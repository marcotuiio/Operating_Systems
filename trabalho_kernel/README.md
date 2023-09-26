# Tutorial para Adicinar uma syscall, Recompliar o kernel e Testar a syscall

## Marco Túlio Alves de Barros - 202100560105
## Univesidade Estadual de Londrina - 23/09/2023

* Não consegui compilar a 6.5.4, então usei a 4.17.4 no Xubuntu 16.04 LTS 64 bits.

1. Baixar a versão fonte do kernel em uso:

    Para tal rode os seguintes comandos:

    ```bash
    wget https://www.kernel.org/pub/linux/kernel/v4.x/linux-4.17.4.tar.xz  
    sudo tar -xvf linux-4.17.4.tar.xz -C/usr/src/
    ```
2. Adicionando a chamada:

    A) No diretório /usr/src/linux-4.17.4/ crie um novo diretório, para fins de organização, com o nome da syscall que deseja criar.
    Nesse caso criaremos a syscall hello. Dentro do novo diretório crie um arquivo com o nome da syscall que deseja criar, nesse caso hello.c. Dentro do arquivo hello.c adicione o seguinte código:

    ```c
    #include <linux/kernel.h>
    #include <linux/syscalls.h>
    #include <linux/printk.h>

    asmlinkage long sys_hello(void) {
        printk("HELLO WORLD MARCO TULIO DO KERNEL\n");
        return 0;
    }
    ```
    B) Depois crie um arquivo Makefile com o seguinte conteúdo:

    ```makefile
    obj-y := hello.o
    ```

    C) Volte para o diretório /usr/src/linux-4.17.4/ e rode o seguinte comando e dentro do Makefile busque pela linha:
    
    ```makefile
    core-y := kernel/ mm/ fs/ ipc/ security/ crypto/ block/ # ou algo parecido
    ```
    e adicione o nome do diretório criado anteriormente, nesse caso hello, ficando assim:

    ```makefile
    core-y := kernel/ mm/ fs/ ipc/ security/ crypto/ block/ hello/ # ou algo parecido
    ```

    D) Agora precisamos adicionar a chamada no arquivo syscall_64.tbl. Para tal rode o seguinte comando:

    ```bash
    sudo nano /usr/src/linux-4.17.4/arch/x86/entry/syscalls/syscall_64.tbl
    ```
    Adicione a seguinte linha no final do arquivo, na parte de 64 bits (ou 32 bits) (no meu caso a ultima linha foi 332 então adicionei 333)

    ```bash
    {ultima_linha+1}     64      hello                   sys_hello
    ```
    
    E) Agora precisamos adicionar a chamada no arquivo syscalls.h. Para tal rode o seguinte comando:

    ```bash
    sudo nano include/linux/syscalls.h
    ```
    Adicione a seguinte linha no final do arquivo:

    ```bash
    asmlinkage long sys_hello(void);
    ```

3. Compilação do kernel.

    A) Volte para o diretório /usr/src/linux-4.17.4/ e rode o seguinte comando:

    ```bash
    sudo make menuconfig
    ```
    No menu, use as setinhas para ir em save e aperte enter. Depois aperte enter novamente para confirmar o nome do arquivo e depois aperte enter novamente para sair do menu.

    B) Agora vamos compilar o kernel. Para tal rode o seguinte comando:

    ```bash
    sudo make
    ``` 
    DICA - use o comando make -jn, em que n é a qntd de núcleos do seu processador, para compilar mais rápido.
	
    (OBS) SE der um erro falando "No rule to make target 'debian/canonical-certs.pem', needed by 'certs/x509_certificate_list'.  Stop." você deve abrir o arquivo .config, buscar pelo bloco "Certificates for signature checking" e trocar os itens como a seguir:
    CONFIG_SYSTEM_TRUSTED_KEYS="/usr/local/src/debian/canonical-certs.pem" -> CONFIG_SYSTEM_TRUSTED_KEYS=""
    e
    CONFIG_SYSTEM_REVOCATION_KEYS="/usr/local/src/debian/canonical-revoked-certs.pem" ->  CONFIG_SYSTEM_REVOCATION_KEYS=""

    Esse comando pode demorar um pouco, então tenha paciência.

    C) Agora vamos instalar o kernel. Para tal rode o seguinte comando:

    ```bash
    sudo make modules_install install
    ```
    Esse comando pode demorar um pouco, então tenha paciência.
    
    D) Após a instalação, reinicie o computador. (reboot)

4. Testando a syscall.

    Se tudo deu certo, podemos testar a syscall. Para crie um .c com o conteúdo abaixo:

    ```c
    #include <stdio.h>
    #include <linux/kernel.h>
    #include <sys/syscall.h>
    #include <unistd.h>
    int main()
    {
            long int amma = syscall({numero_da_syscall}); // numero_da_syscall é o numero adicionado em syscall_64.tbl
            printf("System call sys_hello returned %ld\n", amma);
            return 0;
    }
    ```

    Se após executar o .c a mensagem "System call sys_hello returned 0" aparecer, então a syscall foi adicionada com sucesso.
    Para ver a mensagem "Hello world" rode o seguinte comando:

    ```bash
    dmesg
    ```
    E tudo deve estar de acordo.

* OBS: Não consegui usar o tutorial do classroom pois o arquivo /usr/src/linux-6.x/include/asm-i386/unistd.h. não existe (acho que é por que deveria ser para sistemas 32 bits e o meu é 64 bits, não sei). A diferença do estilo de syscall que usei é que preciso chamar a syscall pelo número dela, e não direatamente pelo nome, mas acho que não tem problema.

Nessa pasta os arquivos criados estão nos mesmos diretórios de origem no kernel, para facilitar a compreensão.
Estou enviado também o log do kernel para fins de completude, e na última linha está a mensagem que minha syscall gerou.
