/* menssages.h
 *
 *   Fabio Costa fabiomcosta@dcc.ufba.br
 *   Jundaí Abdon 
 *
 *
 * 
 */

/********** SERVER MESSAGES **********/


// <HOUR>:<MINUTES> \t <CLIENT_NAME> \t conectado.
#define MSG_SRV_CLIENT_CONNECTED "%s \t %s \t Connected.\n"

//<Time> \t <CLIENT_NAME> \t Desconectado.
#define MSG_SRV_CLIENT_DESCONNECTED "%s \t %s \t Desconnected.\n"

//<TIME> \t <CLIENT_NAME> \t <COMMAND> \t Executado:<Sim\Não>
#define MSG_SRV_COMMAND "%s \t %s \t %s \t Executed: %s.\n"



/********** CLIENT MESSAGES **************/

#define MSG_CL_CONNECTED "Successfully connected\n"

// Falha se ja existe um cliente conectado
#define MSG_CL_FAIL "Falha: %s em uso.\n"



