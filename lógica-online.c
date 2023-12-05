if client

if (is_connected) {
    // Aguarda todas as conexões em loop em thread e as trata de acordo com o header.
    
    // Headers:
    // - BombData: Recebe as coordenadas e o timestamp de uma bomba. (Enviado em eventos específicos)
    // - PlayerData: Recebe a posição do player, sua vida, e a última tecla de movimento pressionada. (Enviado em loop)
    // - Timer: Recebe o tempo do mapa. (Enviado em loop)
    
} else {
    // Envia uma resposta com o header Connect contendo o dado g->player.name
    // Aguarda a resposta do servidor
    // Recebe a resposta e a salva em g->player2.name
    // Atualiza a variável is_connected de 0 para 1
}

Connect
["Player 1"]

PlayerData
[0.00 0.00 0 KEY_NULL]

Timer
[1701781200]

BombData
[0.00 0.00 1701781200]

if server

if (is_connected) {
    // Aguarda todas as conexões em loop em thread e as trata de acordo com o header.
    
    // Headers:
    // - BombData: Recebe as coordenadas e o timestamp de uma bomba.
    // - PlayerData: Recebe a posição do player e sua vida.
    // - Timer: Recebe o tempo do mapa.
    
} else {
    // Espera (em thread) uma requisição com o header Connect
    // Ao receber, salva o dado g->player.name em g->player2.name
    // Envia uma resposta com seu player name
    // Atualiza a variável is_connected de 0 para 1
}

if (colocando bomba) {
    // Lógica para colocar uma bomba;
    
    // Se conectado, envia uma mensagem com o header BombData, contendo a posição da bomba ativada e seu timestamp.
    // Exemplo: "BombData\n[200.00 250.00 1701781200]"
}

void updateData(void *args) {
    while(1) {
        // Recebe a mensagem e separa em duas variáveis, header e data
        // Atualiza os dados do player se o header for PlayerData
        // Atualiza o timer do mapa e calcula o ping se o header for Timer
        // Chama uma função para colocar a bomba, se o header for BombData
    }
}
