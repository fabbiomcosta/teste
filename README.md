Program: server_chat.c and client_chat.c
Students: Fabio Costa <fabiomcosta@dcc.ufba.br> and Jundaí Abdon <jundai@dcc.ufba.br>
Date: 26th November, 2015
Professor: Paul Pregnier


 1. Como gerar os binários:

#> make

 2. Sobre o programa server_chat.c

O server_chat recebe um unico argumento. Uma lista encadeada foi criada contendo como nó uma estrutura de dados:  socket e nome do cliente. 
Para as mensagens trocadas entre servidor e clientes temos uma estrutura de dados chamada PACKET contendo: comando, nome e mensagem.

No servidor tem duas threads (T1 e T2). T2 recebe as conexões feitas no T1 atraves da chamada pipe. T2 utiliza a chamada select() para gerenciar as conexões.
Além disso, o servidor implementa mutex para proteger a região compartilhada.

3. Sobre o programa client_chat.c

O cliente recebe três argumentos. Quando o cliente recebe uma mensagem do servidor, ele a imprime
imediatamente na tela. 

