/* menssages.h
 *
 *   Fabio Costa fabiomcosta@dcc.ufba.br
 *   Jundaí Abdon 
 *
 *
 * 
 */

/********** SERVER MESSAGES **********/

#define MSG_SRV_PORT "\nServer's listening on port %i ...\n"

// <HOUR>:<MINUTES> \t <CLIENT_NAME> \t conectado.
#define MSG_SRV_CLIENT_CONNECTED "%s \t %s \t Connected.\n"

//<Time> \t <CLIENT_NAME> \t Desconectado.
#define MSG_SRV_CLIENT_DESCONNECTED "%s \t %s \t Desconnected.\n"

//<TIME> \t <CLIENT_NAME> \t <COMMAND> \t Executado:<Sim\Não>
#define MSG_SRV_COMMAND "%s \t %s \t %s \t Executado: %s.\n"



/********** CLIENT MESSAGES **************/

#define MSG_CL_CONNECTED "Conectado com Sucesso\n"

// Falha se ja existe um cliente conectado
#define MSG_CL_FAIL "Falha: %s em uso.\n"



