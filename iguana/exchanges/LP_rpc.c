
/******************************************************************************
 * Copyright © 2014-2017 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/
//
//  LP_rpc.c
//  marketmaker
//

char *LP_issue_curl(char *debugstr,char *destip,uint16_t port,char *url)
{
    char *retstr = 0; int32_t maxerrs; struct LP_peerinfo *peer = 0;
    peer = LP_peerfind((uint32_t)calc_ipbits(destip),port);
    maxerrs = LP_MAXPEER_ERRORS;
    if ( peer == 0 || (peer->errors < maxerrs || peer->good >= LP_MINPEER_GOOD) )
    {
        if ( (retstr= issue_curlt(url,LP_HTTP_TIMEOUT)) == 0 )
        {
            if ( peer != 0 )
            {
                peer->errors++;
                peer->good *= LP_PEERGOOD_ERRORDECAY;
            } else printf("%s error on (%s:%u) without peer\n",debugstr,destip,port);
        }
        else if ( peer != 0 )
            peer->good++;
    }
    return(retstr);
}

char *LP_isitme(char *destip,uint16_t destport)
{
    if ( LP_mypeer != 0 && strcmp(destip,LP_mypeer->ipaddr) == 0 && LP_mypeer->port == destport )
    {
        //printf("no need to notify ourselves\n");
        return(clonestr("{\"result\":\"success\"}"));
    } else return(0);
}

char *issue_LP_getpeers(char *destip,uint16_t destport,char *ipaddr,uint16_t port,int32_t numpeers,int32_t numutxos)
{
    char url[512],*retstr;
    sprintf(url,"http://%s:%u/api/stats/getpeers?ipaddr=%s&port=%u&numpeers=%d&numutxos=%d",destip,destport,ipaddr,port,numpeers,numutxos);
    retstr = LP_issue_curl("getpeers",destip,port,url);
    //printf("%s -> getpeers.(%s)\n",destip,retstr);
    return(retstr);
}

char *issue_LP_numutxos(char *destip,uint16_t destport,char *ipaddr,uint16_t port,int32_t numpeers,int32_t numutxos)
{
    char url[512],*retstr;
    printf("deprecated issue_LP_numutxos\n");
    return(0);
    sprintf(url,"http://%s:%u/api/stats/numutxos?ipaddr=%s&port=%u&numpeers=%d&numutxos=%d",destip,destport,ipaddr,port,numpeers,numutxos);
    retstr = LP_issue_curl("numutxos",destip,port,url);
    //printf("%s -> getpeers.(%s)\n",destip,retstr);
    return(retstr);
}

char *issue_LP_getutxos(char *destip,uint16_t destport,char *coin,int32_t lastn,char *ipaddr,uint16_t port,int32_t numpeers,int32_t numutxos)
{
    char url[512];
    printf("deprecated issue_LP_getutxos\n");
    return(0);
    sprintf(url,"http://%s:%u/api/stats/getutxos?coin=%s&lastn=%d&ipaddr=%s&port=%u&numpeers=%d&numutxos=%d",destip,destport,coin,lastn,ipaddr,port,numpeers,numutxos);
    return(LP_issue_curl("getutxos",destip,destport,url));
    //return(issue_curlt(url,LP_HTTP_TIMEOUT));
}

char *issue_LP_clientgetutxos(char *destip,uint16_t destport,char *coin,int32_t lastn)
{
    char url[512];//,*retstr;
    printf("deprecated issue_LP_clientgetutxos\n");
    return(0);
    sprintf(url,"http://%s:%u/api/stats/getutxos?coin=%s&lastn=%d&ipaddr=127.0.0.1&port=0",destip,destport,coin,lastn);
    return(LP_issue_curl("clientgetutxos",destip,destport,url));
    //retstr = issue_curlt(url,LP_HTTP_TIMEOUT);
    //printf("%s clientgetutxos.(%s)\n",url,retstr);
    //return(retstr);
}

char *issue_LP_notify(char *destip,uint16_t destport,char *ipaddr,uint16_t port,int32_t numpeers,int32_t numutxos,uint32_t sessionid)
{
    char url[512],*retstr;
    if ( (retstr= LP_isitme(destip,destport)) != 0 )
        return(retstr);
    sprintf(url,"http://%s:%u/api/stats/notify?ipaddr=%s&port=%u&numpeers=%d&numutxos=%d&session=%u",destip,destport,ipaddr,port,numpeers,numutxos,sessionid);
    return(LP_issue_curl("notify",destip,destport,url));
    //return(issue_curlt(url,LP_HTTP_TIMEOUT));
}

char *issue_LP_getprices(char *destip,uint16_t destport)
{
    char url[512];
    sprintf(url,"http://%s:%u/api/stats/getprices",destip,destport);
    //printf("getutxo.(%s)\n",url);
    return(LP_issue_curl("getprices",destip,destport,url));
    //return(issue_curlt(url,LP_HTTP_TIMEOUT));
}

char *issue_LP_listunspent(char *destip,uint16_t destport,char *symbol,char *coinaddr)
{
    char url[512];
    sprintf(url,"http://%s:%u/api/stats/listunspent?coin=%s&address=%s",destip,destport,symbol,coinaddr);
    //printf("listunspent.(%s)\n",url);
    return(LP_issue_curl("listunspent",destip,destport,url));
}

char *LP_apicall(struct iguana_info *coin,char *method,char *params)
{
    cJSON *retjson; char *retstr;
    if ( coin == 0 )
        return(0);
    if ( coin->electrum != 0 )
    {
        if ( (retjson= electrum_submit(coin->symbol,coin->electrum,&retjson,method,params,LP_HTTP_TIMEOUT)) != 0 )
        {
            retstr = jprint(retjson,0);
            //printf("got.%p (%s)\n",retjson,retstr);
            return(retstr);
        } return(clonestr("{\"error\":\"electrum no response\"}"));
    } else return(bitcoind_passthru(coin->symbol,coin->serverport,coin->userpass,method,params));
}

cJSON *bitcoin_json(struct iguana_info *coin,char *method,char *params)
{
    cJSON *retjson = 0; char *retstr;
    // "getinfo", "getrawmempool", "paxprice", "gettxout", "getrawtransaction", "getblock", "listunspent", "listtransactions", "validateaddress", "importprivkey"
    // bitcoind_passthru callers: "importaddress", "estimatefee", "getblockhash", "sendrawtransaction", "signrawtransaction"
    if ( coin != 0 )
    {
        //printf("issue.(%s, %s, %s, %s, %s)\n",coin->symbol,coin->serverport,coin->userpass,method,params);
        if ( coin->electrum != 0 && (strcmp(method,"getblock") == 0 || strcmp(method,"paxprice") == 0 || strcmp(method,"getrawmempool") == 0) )
            return(cJSON_Parse("{\"error\":\"illegal electrum call\"}"));
        if ( coin->inactive == 0 || strcmp(method,"importprivkey") == 0  || strcmp(method,"validateaddress") == 0 )
        {
            if ( coin->electrum == 0 )
            {
                retstr = bitcoind_passthru(coin->symbol,coin->serverport,coin->userpass,method,params);
                if ( retstr != 0 && retstr[0] != 0 )
                {
                    //printf("%s: %s.%s -> (%s)\n",coin->symbol,method,params,retstr);
                    retjson = cJSON_Parse(retstr);
                    free(retstr);
                }
            }
            else
            {
                if ( (retjson= electrum_submit(coin->symbol,coin->electrum,&retjson,method,params,LP_HTTP_TIMEOUT)) != 0 )
                {
                //printf("electrum %s.%s -> (%s)\n",method,params,jprint(retjson,0));
                    /*if ( (resultjson= jobj(retjson,"result")) != 0 )
                    {
                        resultjson = jduplicate(resultjson);
                        free_json(retjson);
                        retjson = resultjson;
                    }*/
                }
            }
        } else retjson = cJSON_Parse("{\"result\":\"disabled\"}");
    } else printf("bitcoin_json cant talk to NULL coin\n");
    return(retjson);
}

void LP_unspents_mark(char *symbol,cJSON *vins)
{
    printf("LOCK (%s)\n",jprint(vins,0));
}

char *NXTnodes[] = { "62.75.159.113", "91.44.203.238", "82.114.88.225", "78.63.207.76", "188.174.110.224", "91.235.72.49", "213.144.130.91", "209.222.98.250", "216.155.128.10", "178.33.203.157", "162.243.122.251", "69.163.47.173", "193.151.106.129", "78.94.2.74", "192.3.196.10", "173.33.112.87", "104.198.173.28", "35.184.154.126", "174.140.167.239", "23.88.113.131", "198.71.84.173", "178.150.207.53", "23.88.61.53", "192.157.233.106", "192.157.241.212", "23.89.192.88", "23.89.200.27", "192.157.241.139", "23.89.200.63", "23.89.192.98", "163.172.214.102", "176.9.85.5", "80.150.243.88", "80.150.243.92", "80.150.243.98", "109.70.186.198", "146.148.84.237", "104.155.56.82", "104.197.157.140", "37.48.73.249", "146.148.77.226", "84.57.170.200", "107.161.145.131", "80.150.243.97", "80.150.243.93", "80.150.243.100", "80.150.243.95", "80.150.243.91", "80.150.243.99", "80.150.243.96", "93.231.187.177", "212.237.23.85", "35.158.179.254", "46.36.66.41", "185.170.113.79", "163.172.68.112", "78.47.35.210", "77.90.90.75", "94.177.196.134", "212.237.22.215", "94.177.234.11", "167.160.180.199", "54.68.189.9", "94.159.62.14", "195.181.221.89", "185.33.145.94", "195.181.209.245", "195.181.221.38", "195.181.221.162", "185.33.145.12", "185.33.145.176", "178.79.128.235", "94.177.214.120", "94.177.199.41", "94.177.214.200", "94.177.213.201", "212.237.13.162", "195.181.221.236", "195.181.221.185", "185.28.103.187", "185.33.146.244", "217.61.123.71", "195.181.214.45", "195.181.212.99", "195.181.214.46", "195.181.214.215", "195.181.214.68", "217.61.123.118", "195.181.214.79", "217.61.123.14", "217.61.124.100", "195.181.214.111", "85.255.0.176", "81.2.254.116", "217.61.123.184", "195.181.212.231", "94.177.214.110", "195.181.209.164", "104.129.56.238", "85.255.13.64", "167.160.180.206", "217.61.123.226", "167.160.180.208", "93.186.253.127", "212.237.6.208", "94.177.207.190", "217.61.123.119", "85.255.1.245", "217.61.124.157", "37.59.57.141", "167.160.180.58", "104.223.53.14", "217.61.124.69", "195.181.212.103", "85.255.13.141", "104.207.133.204", "71.90.7.107", "107.150.18.108", "23.94.134.161", "80.150.243.13", "80.150.243.11", "185.81.165.52", "80.150.243.8" };


cJSON *LP_assethbla(char *assetid)
{
    char url[1024],*retstr; int32_t n; cJSON *array,*bid=0,*ask=0,*retjson;
    sprintf(url,"http://%s:7876/nxt?=%%2Fnxt&requestType=getBidOrders&asset=%s&firstIndex=0&lastIndex=0",NXTnodes[rand() % (sizeof(NXTnodes)/sizeof(*NXTnodes))],assetid);
    if ( (retstr= issue_curlt(url,LP_HTTP_TIMEOUT)) != 0 )
    {
        bid = cJSON_Parse(retstr);
        free(retstr);
    }
    sprintf(url,"http://%s:7876/nxt?=%%2Fnxt&requestType=getAskOrders&asset=%s&firstIndex=0&lastIndex=0",NXTnodes[rand() % (sizeof(NXTnodes)/sizeof(*NXTnodes))],assetid);
    if ( (retstr= issue_curlt(url,LP_HTTP_TIMEOUT)) != 0 )
    {
        ask = cJSON_Parse(retstr);
        free(retstr);
    }
    retjson = cJSON_CreateObject();
    if ( bid != 0 && ask != 0 )
    {
        if ( (array= jarray(&n,bid,"bidOrders")) != 0 )
            jadd(retjson,"bid",jduplicate(jitem(array,0)));
        if ( (array= jarray(&n,ask,"askOrders")) != 0 )
            jadd(retjson,"ask",jduplicate(jitem(array,0)));
    }
    if ( bid != 0 )
        free_json(bid);
    if ( ask != 0 )
        free_json(ask);
    return(retjson);
}

int32_t LP_getheight(struct iguana_info *coin)
{
    cJSON *retjson; char *method = "getinfo"; int32_t height;
    if ( coin == 0 )
        return(-1);
    height = coin->height;
    if ( coin->electrum == 0 && time(NULL) > coin->heighttime+60 )
    {
        if ( strcmp(coin->symbol,"BTC") == 0 )
            method = "getblockchaininfo";
        if ( (retjson= bitcoin_json(coin,method,"[]")) != 0 )
        {
            coin->height = height = jint(retjson,"blocks");
            free_json(retjson);
            coin->heighttime = (uint32_t)time(NULL);
        }
    }
    return(height);
}

cJSON *LP_getmempool(char *symbol,char *coinaddr)
{
    cJSON *array; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 || (coin->electrum != 0 && coinaddr == 0) )
        return(cJSON_Parse("{\"error\":\"no native coin\"}"));
    if ( coin->electrum == 0 )
        return(bitcoin_json(coin,"getrawmempool","[]"));
    else return(electrum_address_getmempool(symbol,coin->electrum,&array,coinaddr));
}

cJSON *LP_paxprice(char *fiat)
{
    char buf[128],lfiat[65]; struct iguana_info *coin = LP_coinfind("KMD");
    strcpy(lfiat,fiat);
    tolowercase(lfiat);
    sprintf(buf,"[\"%s\", \"kmd\"]",lfiat);
    return(bitcoin_json(coin,"paxprice",buf));
}

cJSON *LP_gettx(char *symbol,bits256 txid)
{
    char buf[128],str[65],*hexstr; int32_t len; bits256 checktxid; cJSON *retjson; struct iguana_info *coin; struct iguana_msgtx msgtx; uint8_t *extraspace,*serialized;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 )
        return(cJSON_Parse("{\"error\":\"no coin\"}"));
    if ( bits256_nonz(txid) == 0 )
        return(cJSON_Parse("{\"error\":\"null txid\"}"));
    if ( coin->electrum == 0 )
    {
        sprintf(buf,"[\"%s\", 1]",bits256_str(str,txid));
        return(bitcoin_json(coin,"getrawtransaction",buf));
    }
    else
    {
        sprintf(buf,"[\"%s\"]",bits256_str(str,txid));
        if ( (retjson= bitcoin_json(coin,"blockchain.transaction.get",buf)) != 0 )
        {
            hexstr = jprint(retjson,1);
            if ( strlen(hexstr) > 100000 )
            {
                printf("rawtransaction too big %d\n",(int32_t)strlen(hexstr));
                free(hexstr);
                return(cJSON_Parse("{\"error\":\"transaction too big\"}"));
            }
            if ( hexstr[0] == '"' && hexstr[strlen(hexstr)-1] == '"' )
                hexstr[strlen(hexstr)-1] = 0;
            if ( (len= is_hexstr(hexstr+1,0)) > 2 )
            {
                memset(&msgtx,0,sizeof(msgtx));
                len = (int32_t)strlen(hexstr+1) >> 1;
                serialized = malloc(len);
                decode_hex(serialized,len,hexstr+1);
                free(hexstr);
                //printf("DATA.(%s)\n",hexstr+1);
                extraspace = calloc(1,1000000);
                retjson = bitcoin_data2json(coin->taddr,coin->pubtype,coin->p2shtype,coin->isPoS,coin->height,&checktxid,&msgtx,extraspace,1000000,serialized,len,0,0);
                free(serialized);
                free(extraspace);
                //printf("TX.(%s) match.%d\n",jprint(retjson,0),bits256_cmp(txid,checktxid));
                return(retjson);
            } else printf("non-hex tx.(%s)\n",hexstr);
            free(hexstr);
            return(cJSON_Parse("{\"error\":\"non hex transaction\"}"));
        } else printf("failed blockcjhain.transaction.get\n");
        return(cJSON_Parse("{\"error\":\"no transaction bytes\"}"));
    }
}

cJSON *LP_gettxout(char *symbol,bits256 txid,int32_t vout)
{
    char buf[128],str[65],coinaddr[64],*hexstr; uint64_t value; uint8_t *serialized; cJSON *sobj,*addresses,*item,*array,*hexobj,*retjson=0; int32_t i,n,v,len; bits256 t; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 )
        return(cJSON_Parse("{\"error\":\"no coin\"}"));
    if ( bits256_nonz(txid) == 0 )
        return(cJSON_Parse("{\"error\":\"null txid\"}"));
    if ( coin->electrum == 0 )
    {
        sprintf(buf,"[\"%s\", %d, true]",bits256_str(str,txid),vout);
        return(bitcoin_json(coin,"gettxout",buf));
    }
    else
    {
        printf("gettxout v%d\n",vout);
        sprintf(buf,"[\"%s\"]",bits256_str(str,txid));
        if ( (hexobj= bitcoin_json(coin,"blockchain.transaction.get",buf)) != 0 )
        {
            hexstr = jprint(hexobj,1);
            if ( strlen(hexstr) > 50000 )
            {
                printf("rawtransaction too big %d\n",(int32_t)strlen(hexstr));
                free(hexstr);
                return(cJSON_Parse("{\"error\":\"transaction too big\"}"));
            }
            if ( hexstr[0] == '"' && hexstr[strlen(hexstr)-1] == '"' )
                hexstr[strlen(hexstr)-1] = 0;
            if ( (len= is_hexstr(hexstr+1,0)) > 2 )
            {
                len = (int32_t)strlen(hexstr+1) >> 1;
                serialized = malloc(len);
                decode_hex(serialized,len,hexstr+1);
                LP_swap_coinaddr(coin,coinaddr,&value,serialized,len,vout);
                //printf("HEX.(%s) len.%d %s %.8f\n",hexstr+1,len,coinaddr,dstr(value));
                free(hexstr);
                if ( (array= electrum_address_listunspent(coin->symbol,0,&array,coinaddr)) != 0 )
                {
                    //printf("array.(%s)\n",jprint(array,0));
                    if ( array != 0 && (n= cJSON_GetArraySize(array)) > 0 )
                    {
                        for (i=0; i<n; i++)
                        {
                            item = jitem(array,i);
                            t = jbits256(item,"tx_hash");
                            v = jint(item,"tx_pos");
                            if ( v == vout && bits256_cmp(t,txid) == 0 )
                            {
                                retjson = cJSON_CreateObject();
                                /*{
                                    "bestblock": "002f7bbe3973b735f535d472501962e86ce8dbc76c73ac5a310a905931b907fa",
                                    "confirmations": 7,
                                    "value": 2013.10431750,
                                    "scriptPubKey": {
                                        "asm": "03b7621b44118017a16043f19b30cc8a4cfe068ac4e42417bae16ba460c80f3828 OP_CHECKSIG",
                                        "hex": "2103b7621b44118017a16043f19b30cc8a4cfe068ac4e42417bae16ba460c80f3828ac",
                                        "reqSigs": 1,
                                        "type": "pubkey",
                                        "addresses": [
                                                      "RNJmgYaFF5DbnrNUX6pMYz9rcnDKC2tuAc"
                                                      ]
                                    },
                                    "version": 1,
                                    "coinbase": false
                                }*/
                                if ( value != j64bits(item,"value") )
                                    printf("LP_gettxout: value %llu != %llu\n",(long long)value,(long long)j64bits(item,"value"));
                                jaddnum(retjson,"value",dstr(value));
                                jaddbits256(retjson,"txid",t);
                                jaddnum(retjson,"vout",v);
                                addresses = cJSON_CreateArray();
                                jaddistr(addresses,coinaddr);
                                sobj = cJSON_CreateObject();
                                jaddnum(sobj,"reqSigs",1);
                                jaddstr(sobj,"type","pubkey");
                                jadd(sobj,"addresses",addresses);
                                jadd(retjson,"scriptPubKey",sobj);
                                printf("GETTXOUT.(%s)\n",jprint(retjson,0));
                                break;
                            }
                        }
                    }
                    free_json(array);
                }
            } else free(hexstr);
            return(retjson);
        }
        return(cJSON_Parse("{\"error\":\"couldnt get tx\"}"));
    }
}

/*cJSON *LP_listtransactions(char *symbol,char *coinaddr,int32_t count,int32_t skip)
{
    char buf[128]; struct iguana_info *coin = LP_coinfind(symbol);
    if ( coin == 0 )
        return(cJSON_Parse("{\"error\":\"no coin\"}"));
    if ( count == 0 )
        count = 100;
    sprintf(buf,"[\"%s\", %d, %d, true]",coinaddr,count,skip);
    return(bitcoin_json(coin,"listtransactions",buf));
}*/

cJSON *LP_validateaddress(char *symbol,char *address)
{
    char buf[512],coinaddr[64],checkaddr[64],script[128]; int32_t i; uint8_t rmd160[20],addrtype; cJSON *retjson; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 )
        return(cJSON_Parse("{\"error\":\"no coin\"}"));
    if ( coin != 0 && coin->electrum != 0 )
    {
        retjson = cJSON_CreateObject();
        jaddstr(retjson,"address",address);
        bitcoin_addr2rmd160(coin->taddr,&addrtype,rmd160,address);
        bitcoin_address(checkaddr,coin->taddr,addrtype,rmd160,20);
        jadd(retjson,"isvalid",strcmp(address,checkaddr)==0? cJSON_CreateTrue() : cJSON_CreateFalse());
        if ( addrtype == coin->pubtype )
        {
            strcpy(script,"76a914");
            for (i=0; i<20; i++)
                sprintf(&script[i*2+6],"%02x",rmd160[i]);
            script[i*2+6] = 0;
            strcat(script,"88ac");
            jaddstr(retjson,"scriptPubKey",script);
        }
        bitcoin_address(coinaddr,coin->taddr,coin->pubtype,LP_myrmd160,20);
        if ( strcmp(address,coinaddr) == 0 )
            jadd(retjson,"ismine",cJSON_CreateTrue());
        jadd(retjson,"iswatchonly",cJSON_CreateFalse());
        jadd(retjson,"isscript",addrtype == coin->p2shtype ? cJSON_CreateTrue() : cJSON_CreateFalse());
        return(retjson);
    }
    else
    {
        sprintf(buf,"[\"%s\"]",address);
        return(bitcoin_json(coin,"validateaddress",buf));
    }
}

int32_t LP_address_ismine(char *symbol,char *address)
{
    int32_t doneflag = 0; cJSON *retjson;
    if ( symbol == 0 || symbol[0] == 0 )
        return(0);
    if ( (retjson= LP_validateaddress(symbol,address)) != 0 )
    {
        if ( jobj(retjson,"ismine") != 0 && is_cJSON_True(jobj(retjson,"ismine")) != 0 )
        {
            doneflag = 1;
            //printf("%s already ismine\n",address);
        }
        //printf("%s\n",jprint(retjson,0));
        free_json(retjson);
    }
    return(doneflag);
}

cJSON *LP_listunspent(char *symbol,char *coinaddr)
{
    char buf[128]; cJSON *retjson; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    //printf("LP_listunspent.(%s %s)\n",symbol,coinaddr);
    if ( coin == 0 || coin->inactive != 0 )
        return(cJSON_Parse("{\"error\":\"no coin\"}"));
    if ( coin->electrum == 0 )
    {
        if ( LP_address_ismine(symbol,coinaddr) > 0 )
        {
            sprintf(buf,"[0, 99999999, [\"%s\"]]",coinaddr);
            return(bitcoin_json(coin,"listunspent",buf));
        } else return(LP_address_utxos(coin,coinaddr,0));
    } else return(electrum_address_listunspent(symbol,coin->electrum,&retjson,coinaddr));
}

void LP_listunspent_issue(char *symbol,char *coinaddr)
{
    struct iguana_info *coin; cJSON *retjson=0; char *retstr=0,destip[64]; uint16_t destport;
    if ( symbol == 0 || symbol[0] == 0 )
        return;
    if ( (coin= LP_coinfind(symbol)) != 0 )
    {
        if ( coin->electrum != 0 )
            retjson = electrum_address_listunspent(symbol,coin->electrum,&retjson,coinaddr);
        else
        {
            if ( (destport= LP_randpeer(destip)) > 0 )
            {
                retstr = issue_LP_listunspent(destip,destport,symbol,coinaddr);
                if ( (retjson= cJSON_Parse(retstr)) != 0 )
                {
                    if ( electrum_process_array(coin,coinaddr,retjson) != 0 )
                    {
                        LP_postutxos(symbol,coinaddr); // might be good to not saturate
                    }
                }
                //printf("rand %s listunspent.(%s) to %s:%u -> %s\n",symbol,coinaddr,destip,destport,retstr);
            }
        }
        if ( retjson != 0 )
            free_json(retjson);
        if ( retstr != 0 )
            free(retstr);
    }
}

cJSON *LP_importprivkey(char *symbol,char *wifstr,char *label,int32_t flag)
{
    static void *ctx;
    char buf[512],address[64]; cJSON *retjson; struct iguana_info *coin; int32_t doneflag = 0;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 )
        return(cJSON_Parse("{\"error\":\"no coin\"}"));
    if ( coin->electrum != 0 )
        return(cJSON_Parse("{\"result\":\"electrum should have local wallet\"}"));
    if ( ctx == 0 )
        ctx = bitcoin_ctx();
    bitcoin_wif2addr(ctx,coin->wiftaddr,coin->taddr,coin->pubtype,address,wifstr);
    if ( (retjson= LP_validateaddress(symbol,address)) != 0 )
    {
        if ( jobj(retjson,"ismine") != 0 && is_cJSON_True(jobj(retjson,"ismine")) != 0 )
        {
            doneflag = 1;
            //printf("%s already ismine\n",address);
        }
        //printf("%s\n",jprint(retjson,0));
        free_json(retjson);
    }
    if ( doneflag == 0 )
    {
        if ( coin->noimportprivkey_flag != 0 )
            sprintf(buf,"[\"%s\"]",wifstr);
        else sprintf(buf,"\"%s\", \"%s\", %s",wifstr,label,flag < 0 ? "false" : "true");
        return(bitcoin_json(coin,"importprivkey",buf));
    } else return(cJSON_Parse("{\"result\":\"success\"}"));
}

int32_t LP_importaddress(char *symbol,char *address)
{
    char buf[1024],*retstr; cJSON *validatejson; int32_t isvalid=0,doneflag = 0; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(-2);
    coin = LP_coinfind(symbol);
    if ( coin == 0 )
        return(-2);
    if ( coin->electrum != 0 )
    {
        /*if ( (retjson= electrum_address_subscribe(symbol,coin->electrum,&retjson,address)) != 0 )
        {
            printf("importaddress.(%s) -> %s\n",address,jprint(retjson,0));
            free_json(retjson);
        }*/
        return(0);
    }
    else
    {
        if ( (validatejson= LP_validateaddress(symbol,address)) != 0 )
        {
            if ( (isvalid= is_cJSON_True(jobj(validatejson,"isvalid")) != 0) != 0 )
            {
                if ( is_cJSON_True(jobj(validatejson,"iswatchonly")) != 0 || is_cJSON_True(jobj(validatejson,"ismine")) != 0 )
                    doneflag = 1;
            }
            free_json(validatejson);
        }
        if ( isvalid == 0 )
            return(-1);
        if ( doneflag != 0 )
            return(0); // success
        sprintf(buf,"[\"%s\", \"%s\", false]",address,address);
        if ( (retstr= bitcoind_passthru(symbol,coin->serverport,coin->userpass,"importaddress",buf)) != 0 )
        {
            //printf("importaddress.(%s %s) -> (%s)\n",symbol,address,retstr);
            free(retstr);
        } //else printf("importaddress.(%s %s)\n",symbol,address);
        return(1);
    }
}

double LP_getestimatedrate(struct iguana_info *coin)
{
    char buf[512],*retstr; double rate = 0.00000020;
    if ( coin == 0 )
        return(0.0001);
    if ( (strcmp(coin->symbol,"BTC") == 0 || coin->txfee == 0) )
    {
        if ( coin->rate == 0. || time(NULL) > coin->ratetime+60 )
        {
            sprintf(buf,"[%d]",6);
            if ( (retstr= LP_apicall(coin,coin->electrum==0?"estimatefee" : "blockchain.estimatefee",buf)) != 0 )
            {
                if ( retstr[0] != '-' )
                {
                    rate = atof(retstr) / 1024.;
                    if ( rate < 0.00000020 )
                        rate = 0.00000020;
                    coin->rate = rate * 1.25;
                    coin->ratetime = (uint32_t)time(NULL);
                    if ( (rand() % 10) == 0 )
                        printf("estimated rate.(%s) %s -> %.8f\n",coin->symbol,retstr,rate);
                }
                free(retstr);
            }
        } else rate = coin->rate;
    } else return((double)coin->txfee / LP_AVETXSIZE);
    return(SATOSHIDEN * rate);
}

char *LP_sendrawtransaction(char *symbol,char *signedtx)
{
    cJSON *array; char *paramstr,*tmpstr,*retstr=0; int32_t n; cJSON *retjson; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(0);
    coin = LP_coinfind(symbol);
    if ( coin == 0 )
        return(0);
    if ( coin->electrum == 0 )
    {
        array = cJSON_CreateArray();
        jaddistr(array,signedtx);
        paramstr = jprint(array,1);
        retstr = bitcoind_passthru(symbol,coin->serverport,coin->userpass,"sendrawtransaction",paramstr);
        //printf(">>>>>>>>>>> %s dpow_sendrawtransaction.(%s) -> (%s)\n",coin->symbol,paramstr,retstr);
        free(paramstr);
    }
    else
    {
        if ( (retjson= electrum_sendrawtransaction(symbol,coin->electrum,&retjson,signedtx)) != 0 )
        {
            retstr = jprint(retjson,1);
            printf("electrum sendrawtx.(%s) -> %s\n",signedtx,retstr);
            n = (int32_t)strlen(retstr);
            if ( retstr[0] == '"' && retstr[n-1] == '"' )
            {
                retstr[n-1] = 0;
                tmpstr = clonestr(retstr+1);
                free(retstr);
                retstr = tmpstr;
            }
        }
    }
    return(retstr);
}

char *LP_signrawtx(char *symbol,bits256 *signedtxidp,int32_t *completedp,cJSON *vins,char *rawtx,cJSON *privkeys,struct vin_info *V)
{
    cJSON *array,*json,*retjson; int32_t len; uint8_t *data; char str[65],*paramstr,*retstr,*hexstr,*signedtx=0; struct iguana_msgtx msgtx; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(0);
    coin = LP_coinfind(symbol);
    memset(signedtxidp,0,sizeof(*signedtxidp));
    *completedp = 0;
    if ( coin == 0 )
    {
        printf("LP_signrawtx cant find coin.(%s)\n",symbol);
        return(0);
    }
    //int32_t iguana_signrawtransaction(void *ctx,char *symbol,uint8_t wiftaddr,uint8_t taddr,uint8_t pubtype,uint8_t p2shtype,uint8_t isPoS,int32_t height,struct iguana_msgtx *msgtx,char **signedtxp,bits256 *signedtxidp,struct vin_info *V,int32_t numinputs,char *rawtx,cJSON *vins,cJSON *privkeysjson)
    memset(&msgtx,0,sizeof(msgtx));
    signedtx = 0;
    memset(signedtxidp,0,sizeof(*signedtxidp));
    //printf("locktime.%u sequenceid.%x rawtx.(%s) vins.(%s)\n",locktime,sequenceid,rawtxbytes,jprint(vins,0));
    if ( (*completedp= iguana_signrawtransaction(coin->ctx,symbol,coin->wiftaddr,coin->taddr,coin->pubtype,coin->p2shtype,coin->isPoS,1000000,&msgtx,&signedtx,signedtxidp,V,16,rawtx,vins,privkeys)) < 0 )
        //if ( (signedtx= LP_signrawtx(symbol,signedtxidp,&completed,vins,rawtxbytes,privkeys,V)) == 0 )
        printf("couldnt sign transaction.%s %s\n",rawtx,bits256_str(str,*signedtxidp));
    else if ( *completedp == 0 )
    {
        printf("incomplete signing %s (%s)\n",rawtx,jprint(vins,0));
        if ( signedtx != 0 )
            free(signedtx), signedtx = 0;
    } else printf("basilisk_swap_bobtxspend %s -> %s\n",rawtx,bits256_str(str,*signedtxidp));
    if ( signedtx == 0 )
    {
        retjson = cJSON_CreateObject();
        jaddstr(retjson,"error","couldnt sign tx");
        jaddstr(retjson,"coin",coin->symbol);
        jaddstr(retjson,"rawtx",rawtx);
        jadd(retjson,"vins",vins);
        jadd(retjson,"privkeys",privkeys);
        return(jprint(retjson,1));
    }
    return(signedtx);
    
    array = cJSON_CreateArray();
    jaddistr(array,rawtx);
    jaddi(array,jduplicate(vins));
    jaddi(array,jduplicate(privkeys));
    paramstr = jprint(array,1);
    //printf("signrawtransaction\n");
    if ( (retstr= bitcoind_passthru(symbol,coin->serverport,coin->userpass,"signrawtransaction",paramstr)) != 0 )
    {
        if ( (json= cJSON_Parse(retstr)) != 0 )
        {
            if ( (hexstr= jstr(json,"hex")) != 0 )
            {
                len = (int32_t)strlen(hexstr);
                signedtx = calloc(1,len+1);
                strcpy(signedtx,hexstr);
                *completedp = is_cJSON_True(jobj(json,"complete"));
                len >>= 1;
                data = malloc(len);
                decode_hex(data,len,hexstr);
                *signedtxidp = bits256_doublesha256(0,data,len);
            }
            //else
            printf("%s signrawtransaction.(%s) params.(%s)\n",coin->symbol,retstr,paramstr);
            free_json(json);
        }
        free(retstr);
    }
    free(paramstr);
    return(signedtx);
}

cJSON *LP_getblock(char *symbol,bits256 txid)
{
    char buf[128],str[65]; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 || coin->electrum != 0 )
        return(cJSON_Parse("{\"error\":\"no native coin\"}"));
    sprintf(buf,"[\"%s\"]",bits256_str(str,txid));
    return(bitcoin_json(coin,"getblock",buf));
}

// not in electrum path
uint64_t LP_txfee(char *symbol)
{
    uint64_t txfee = 0;
    if ( symbol == 0 || symbol[0] == 0 )
        return(LP_MIN_TXFEE);
    if ( strcmp(symbol,"BTC") != 0 )
        txfee = LP_MIN_TXFEE;
    return(txfee);
}

char *LP_blockhashstr(char *symbol,int32_t height)
{
    cJSON *array; char *paramstr,*retstr; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(0);
    coin = LP_coinfind(symbol);
    if ( coin == 0 || coin->electrum != 0 )
        return(0);
    array = cJSON_CreateArray();
    jaddinum(array,height);
    paramstr = jprint(array,1);
    retstr = bitcoind_passthru(symbol,coin->serverport,coin->userpass,"getblockhash",paramstr);
    free(paramstr);
    return(retstr);
}

cJSON *LP_getblockhashstr(char *symbol,char *blockhashstr)
{
    char buf[128]; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 || coin->electrum != 0 )
        return(cJSON_Parse("{\"error\":\"no native coin daemon\"}"));
    sprintf(buf,"[\"%s\"]",blockhashstr);
    return(bitcoin_json(coin,"getblock",buf));
}

cJSON *LP_blockjson(int32_t *heightp,char *symbol,char *blockhashstr,int32_t height)
{
    cJSON *json = 0; int32_t flag = 0; struct iguana_info *coin;
    if ( symbol == 0 || symbol[0] == 0 )
        return(cJSON_Parse("{\"error\":\"null symbol\"}"));
    coin = LP_coinfind(symbol);
    if ( coin == 0 || coin->electrum != 0 )
    {
        printf("unexpected electrum path for %s\n",symbol);
        return(0);
    }
    if ( blockhashstr == 0 )
        blockhashstr = LP_blockhashstr(symbol,height), flag = 1;
    if ( blockhashstr != 0 )
    {
        if ( (json= LP_getblockhashstr(symbol,blockhashstr)) != 0 )
        {
            if ( *heightp != 0 )
            {
                *heightp = juint(json,"height");
                if ( height >= 0 && *heightp != height )
                {
                    printf("unexpected height %d vs %d for %s (%s)\n",*heightp,height,blockhashstr,jprint(json,0));
                    *heightp = -1;
                    free_json(json);
                    json = 0;
                }
            }
        }
        if ( flag != 0 && blockhashstr != 0 )
            free(blockhashstr);
    }
    return(json);
}

