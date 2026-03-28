# 📝 Explicação da Linguagem HomeScript

A **HomeScript** é uma linguagem de programação criada para automação residencial inteligente. Ela permite que qualquer pessoa, mesmo sem conhecimento de programação, controle dispositivos como lâmpadas, ventiladores e motores usando comandos simples e intuitivos.

O usuário declara seus dispositivos e sensores com `device` e `sensor`, define ações como `turn on` e `turn off`, e cria regras automáticas usando `when` (quando algo acontece) e `if` (se uma condição for verdadeira). O comando `wait` permite adicionar pausas em milissegundos.

O código é escrito em arquivos `.iot` e processado por um compilador que realiza análise léxica (identificação de tokens), análise sintática (validação da estrutura) e geração de código, traduzindo tudo para C/Arduino — pronto para ser executado em microcontroladores reais.

O diferencial da HomeScript é transformar automação residencial em algo acessível: em vez de programar registradores e portas digitais em C, o usuário simplesmente escreve `turn luz on` e o sistema cuida do resto. A linguagem também suporta português (`ligar`, `quando`, `se`), tornando-a ainda mais acessível para usuários brasileiros.
