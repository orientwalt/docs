﻿/************************************************************************
author: yqq
date: 2019-05-08  14:25
descriptions: htdf transaction signature 
************************************************************************/

#include "htdf.h"
#include <secp256k1.h>
#include <secp256k1_recovery.h>

#include "crypto/strencodings.h"
#include "crypto/hash.h"
#include "bech32/bech32.h"
#include "crypto/strencodings.h"
#include "crypto/tinyformat.h"
#include "crypto/string.h"

using namespace htdf;

int htdf::sign(
    unsigned char *pszIn,
    unsigned int uInLen,
    unsigned char *pszPrivKey,
    unsigned int uPrivKeyLen,
    unsigned char *pszOut,
    unsigned int uOutBufLen,
    unsigned int *puOutDataLen,
    char *pszErrMsg)
{
    if (NULL == pszErrMsg)
    {
        return htdf::ARGS_ERROR;
    }

    if (NULL == pszIn)
    {
        strcpy(pszErrMsg, "pszIn is null.");
        return htdf::ARGS_ERROR;
    }

    if (0 == uInLen)
    {
        strcpy(pszErrMsg, "uInLen is 0.");
        return htdf::ARGS_ERROR;
    }

    if (NULL == pszPrivKey)
    {
        strcpy(pszErrMsg, "pszPrivKey is null.");
        return htdf::ARGS_ERROR;
    }

    if (UINT_PRIV_KEY_LEN != uPrivKeyLen)
    {
        sprintf(pszErrMsg, "priv-key len is not %d bytes.", UINT_PRIV_KEY_LEN);
        return htdf::ARGS_ERROR;
    }

    if (NULL == pszOut)
    {
        strcpy(pszErrMsg, "pszOut is null.");
        return htdf::ARGS_ERROR;
    }

    if (uOutBufLen < UINT_SIG_RS_LEN)
    {
        sprintf(pszErrMsg, "uOutBufLen less than %d. Must more than %d.", UINT_SIG_RS_LEN, UINT_SIG_RS_LEN);
        return htdf::ARGS_ERROR;
    }

    if (NULL == puOutDataLen)
    {
        strcpy(pszErrMsg, "puOutDataLen is null");
        return htdf::ARGS_ERROR;
    }

    auto *ctx = GetSecp256k1Ctx();

    secp256k1_ecdsa_recoverable_signature rawSig;
    memset(&rawSig.data, 0, 65);
    if (!secp256k1_ecdsa_sign_recoverable(ctx, &rawSig, pszIn, pszPrivKey, nullptr, nullptr))
    {
        strcpy(pszErrMsg, "secp256k1_ecdsa_sign_recoverable  failed.");
        return htdf::ECCSIGN_STEP1_ERROR;
    }

    int iRecid = 0;
    unsigned char uszSigRSData[UINT_SIG_RS_LEN] = {0};
    memset(uszSigRSData, 0, sizeof(uszSigRSData));
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, uszSigRSData, &iRecid, &rawSig);

    memcpy(pszOut, uszSigRSData, UINT_SIG_RS_LEN);
    *puOutDataLen = UINT_SIG_RS_LEN;

    // no need to destroy
    // secp256k1_context_destroy(const_cast<secp256k1_context*>(ctx));
    return htdf::NO_ERROR;
}

//输入: 十六进制字符串形式的私钥
//输出: 十六进制字符串形式的公钥
int htdf::PrivateKeyToCompressPubKey(const string &strPrivKey, string &strPubKey)
{
    const int PUBK_SIZE = 33;
    string privKey = HexToBin(strPrivKey);
    secp256k1_pubkey pubkey;
    memset(pubkey.data, 0, sizeof(pubkey.data));

    auto *ctx = GetSecp256k1Ctx();

    if (!secp256k1_ec_pubkey_create(ctx, &pubkey, (unsigned char *)privKey.data()))
    {
        return 1;
    }

    unsigned char output[128] = {0};
    memset(output, 0, sizeof(output));
    size_t outputlen = 33;
    secp256k1_ec_pubkey_serialize(ctx, output, &outputlen, &pubkey, SECP256K1_EC_COMPRESSED);
    if (33 != outputlen)
    {
        return 1;
    }

    strPubKey = Bin2HexStr(output, outputlen);
    return 0;
}

CRawTx::CRawTx()
{
    uAccountNumber = INTMAX_MAX;
    memset(szChainId, 0, sizeof(szChainId));
    uFeeAmount = 0;
    memset(szFeeDenom, 0, sizeof(szFeeDenom));
    uGas = 0;
    memset(szMemo, 0, sizeof(szMemo));
    uMsgAmount = 0;
    memset(szMsgDenom, 0, sizeof(szMsgDenom));
    memset(szMsgFrom, 0, sizeof(szMsgFrom));
    memset(szMsgTo, 0, sizeof(szMsgTo));
    uSequence = INTMAX_MAX;

    memset(szData, 0, sizeof(szData));
}

CRawTx::CRawTx(const CRawTx &other)
{
    uAccountNumber = other.uAccountNumber;
    memcpy(szChainId, other.szChainId, sizeof(szChainId));
    uFeeAmount = other.uFeeAmount;
    memcpy(szFeeDenom, other.szFeeDenom, sizeof(szFeeDenom));
    uGas = other.uGas;
    memcpy(szMemo, other.szMemo, sizeof(szMemo));
    uMsgAmount = other.uMsgAmount;

    memcpy(szMsgDenom, other.szMsgDenom, sizeof(szMsgDenom));
    memcpy(szMsgFrom, other.szMsgFrom, sizeof(szMsgFrom));
    memcpy(szMsgTo, other.szMsgTo, sizeof(szMsgTo));
    uSequence = other.uSequence;

    memcpy(szData, other.szData, sizeof(szData));
}

CRawTx &CRawTx::operator=(const CRawTx &other)
{
    if (this == &other)
        return *this;

    uAccountNumber = other.uAccountNumber;
    memcpy(szChainId, other.szChainId, sizeof(szChainId));
    uFeeAmount = other.uFeeAmount;
    memcpy(szFeeDenom, other.szFeeDenom, sizeof(szFeeDenom));
    uGas = other.uGas;
    memcpy(szMemo, other.szMemo, sizeof(szMemo));
    uMsgAmount = other.uMsgAmount;

    memcpy(szMsgDenom, other.szMsgDenom, sizeof(szMsgDenom));
    memcpy(szMsgFrom, other.szMsgFrom, sizeof(szMsgFrom));
    memcpy(szMsgTo, other.szMsgTo, sizeof(szMsgTo));
    uSequence = other.uSequence;

    memcpy(szData, other.szData, sizeof(szData));
    return *this;
}

bool CRawTx::toString(string &strOut)
{
    if (false == checkParams(strOut))
    {
        return false;
    }
    strOut.clear();

    string strJson;

    strJson += "{"; //root

    char buf[2046] = {0};

    //account_number
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\"account_number\":\"%lu\"", uAccountNumber);
    strJson += buf;
    strJson += ",";

    //chain_id
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\"chain_id\":\"%s\"", szChainId);
    strJson += buf;
    strJson += ",";

    //fee
    strJson += "\"fee\":{";
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\"gas_price\":\"%lu\",\"gas_wanted\":\"%lu\"", uFeeAmount, uGas);
    strJson += buf;
    strJson += "}";
    strJson += ",";

    //memo
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\"memo\":\"%s\"", szMemo);
    strJson += buf;
    strJson += ",";

    //msgs
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\"msgs\":[{\"Amount\":[{\"amount\":\"%lu\",\"denom\":\"%s\"}],\"Data\":\"%s\",\"From\":\"%s\",\"GasPrice\":%lu,\"GasWanted\":%lu,\"To\":\"%s\"}]",
            uMsgAmount, szMsgDenom, szData, szMsgFrom, uFeeAmount, uGas, szMsgTo);
    strJson += buf;
    strJson += ",";

    //sequence
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "\"sequence\":\"%lu\"", uSequence);
    strJson += buf;

    strJson += "}"; //root

    strOut = strJson;
    return true;
}

bool CRawTx::checkParams(string &strErrMsg)
{
    if (INTMAX_MAX == uAccountNumber || 0 == uAccountNumber)
    {
        // strErrMsg = tfm::format("invalid `account_number`: %lu , must between %lu and %lu.") % uAccountNumber % 0 % INTMAX_MAX);
        strErrMsg = tfm::format("invalid `account_number`: %lu , must between %lu and %lu.", uAccountNumber, 0, INTMAX_MAX);
        return false;
    }

    if (!(0 == strcmp(STR_MAINCHAIN, szChainId) || 0 == strcmp(STR_TESTCHAIN, szChainId)))
    {
        strErrMsg = tfm::format("invalid `chain_id`: %s, must be '%s'or '%s' . ", szChainId, STR_MAINCHAIN, STR_TESTCHAIN);
        return false;
    }

    if (UINT_MAX_FEE_AMOUNT < uFeeAmount || uFeeAmount < UINT_MIN_FEE_AMOUNT)
    {
        strErrMsg = tfm::format("invalid `fee amount`, must between %d and %d.", UINT_MIN_FEE_AMOUNT, UINT_MAX_FEE_AMOUNT);
        return false;
    }

    if (UINT_MAX_GAS_AMOUNT < uGas || uGas < UINT_MIN_GAS_AMOUNT)
    {
        strErrMsg = tfm::format("invalid `fee gas` : %lu, must between %lu and %lu.", uGas, UINT_MIN_GAS_AMOUNT, UINT_MAX_GAS_AMOUNT);
        return false;
    }

    if (!(0 == strcmp(STR_SATOSHI, szFeeDenom)))
    {
        strErrMsg = tfm::format("invalid `fee denom` : %s, must be `%s`.", szFeeDenom, STR_SATOSHI);
        return false;
    }

    if (!(0 == strcmp(STR_SATOSHI, szMsgDenom)))
    {
        strErrMsg = tfm::format("invalid `msgs amount denom` : %s, must be `%s`.", szMsgDenom, STR_SATOSHI);
        return false;
    }

    int nAddrLen = UINT_ADDR_LEN;
    if (nAddrLen != strlen(szMsgFrom))
    {
        strErrMsg = tfm::format("invalid address `msg From`:%s, address length must be %d.", szMsgFrom, nAddrLen);
        return false;
    }

    if (nAddrLen != strlen(szMsgTo))
    {
        strErrMsg = tfm::format("invalid address `msg To`:%s, address length must be %d.", szMsgTo, nAddrLen);
        return false;
    }

    if (0 != string(szMsgFrom).find(STR_HTDF "1"))
    {
        strErrMsg = tfm::format("invalid address `From`:%s.", szMsgFrom);
        return false;
    }

    if (0 != string(szMsgTo).find(STR_HTDF "1"))
    {
        strErrMsg = tfm::format("invalid address `To`:%s.", szMsgTo);
        return false;
    }

    if (INTMAX_MAX <= uSequence)
    {
        strErrMsg = "invalid `sequence` , TOO LARGE, this is tx number of the address in the node.";
        return false;
    }

    return true;
}

CBroadcastTx::CBroadcastTx()
{
    strType = STR_BROADCAST_TYPE; //"auth/StdTx";
    rtx = CRawTx();
    strMsgType = STR_BROADCAST_MSG_TYPE;        // "htdfservice/send";
    strPubKeyType = STR_BROADCAST_PUB_KEY_TYPE; //"tendermint/PubKeySecp256k1";
    strPubkeyValue = "";
    strSignature = "";
}

CBroadcastTx::CBroadcastTx(const CBroadcastTx &other)
{
    strType = other.strType;
    rtx = other.rtx;
    strMsgType = other.strMsgType;
    strPubKeyType = other.strPubKeyType;
    strPubkeyValue = other.strPubkeyValue;
    strSignature = other.strSignature;
}

CBroadcastTx &CBroadcastTx::operator=(const CBroadcastTx &other)
{
    strType = other.strType;
    rtx = other.rtx;
    strMsgType = other.strMsgType;
    strPubKeyType = other.strPubKeyType;
    strPubkeyValue = other.strPubkeyValue;
    strSignature = other.strSignature;

    return *this;
}

bool CBroadcastTx::toString(string &strRet)
{
    string strErrMsg;
    if (false == checkParams(strErrMsg))
    {
        strRet = strErrMsg;
        return false;
    }

    string strJson;
    strJson += "{"; //root

    //type
    strJson += "\"type\":\"" + strType + "\",";

    //value
    strJson += "\"value\":{";

    //msg
    strJson += tfm::format("\
			\"msg\":[{\
				\"type\":\"%s\",\
				\"value\":{\
					\"From\":\"%s\",\
					\"To\":\"%s\",\
					\"Amount\":[{\
						\"denom\":\"%s\",\
						\"amount\":\"%lu\"\
					}],\
					\"Data\":\"%s\",\
					\"GasPrice\":\"%lu\",\
					\"GasWanted\":\"%lu\"\
				  }\
			}],",
                           strMsgType,
                           rtx.szMsgFrom,
                           rtx.szMsgTo, rtx.szMsgDenom, rtx.uMsgAmount, rtx.szData, rtx.uFeeAmount, rtx.uGas);

    //fee
    strJson += tfm::format("\
				\"fee\":{\
					\"gas_price\":\"%lu\",\
					\"gas_wanted\":\"%lu\"\
				},",
                           rtx.uFeeAmount,
                           rtx.uGas);

    //signatures
    strJson += tfm::format("\
			\"signatures\":[{\
				\"pub_key\":{\
					\"type\":\"%s\",\
					\"value\":\"%s\"\
				},\
				\"signature\":\"%s\"\
			}],",
                           strPubKeyType,
                           strPubkeyValue, strSignature);

    strJson = RemoveSpace(strJson);
    strJson += tfm::format("\"memo\":\"%s\"", rtx.szMemo);

    strJson += "}"; //value
    strJson += "}"; //root

    strRet = strJson;
    return true;
}

bool CBroadcastTx::checkParams(string &strErrMsg)
{

    if (false == rtx.checkParams(strErrMsg))
    {
        strErrMsg += "HtdfBroadcastTx::ParamsCheck:";
        return false;
    }

    if (STR_BROADCAST_MSG_TYPE != strMsgType && STR_BROADCAST_MSG_TYPE_HET != strMsgType)
    {
        strErrMsg = tfm::format("invalid `msg type` : '%s', must be '%s' or '%s'.", strMsgType, STR_BROADCAST_MSG_TYPE, STR_BROADCAST_MSG_TYPE_HET);
        return false;
    }

    if (STR_BROADCAST_PUB_KEY_TYPE != strPubKeyType)
    {
        strErrMsg = tfm::format("invalid `pub_key type` : '%s', must be '%s'.", strPubKeyType, STR_BROADCAST_PUB_KEY_TYPE);
        return false;
    }

    if (strPubkeyValue.empty())
    {

        strErrMsg = tfm::format("invalid `pub_key value` is empty, must be base64(pubkey).", strPubKeyType, STR_BROADCAST_PUB_KEY_TYPE);
        return false;
    }

    string strTmpDecode;

    strTmpDecode = DecodeBase64(strPubkeyValue);
    if (UINT_PUB_KEY_LEN != strTmpDecode.size())
    {
        strErrMsg = tfm::format("invalid `pub_key value` length %d is not %d. After base64 decode, pubkey's length must be %d.", strTmpDecode.size(), UINT_PUB_KEY_LEN, UINT_PUB_KEY_LEN);
        return false;
    }

    strTmpDecode.clear();
    strTmpDecode = DecodeBase64(strSignature);
    if (UINT_SIG_RS_LEN != strTmpDecode.size())
    {
        strErrMsg = tfm::format("invalid `signature` length is not %d. After base64 decode, signature's length must be %d.", UINT_SIG_RS_LEN, UINT_SIG_RS_LEN);
        return false;
    }

    return true;
}

bool CBroadcastTx::toHexStr(string &strOut)
{
    string strErrMsg;
    if (false == checkParams(strErrMsg))
    {
        strOut = strErrMsg;
        return false;
    }

    string strRet;
    string strHex;
    if (false == toString(strErrMsg))
    {
        strOut = strErrMsg;
        return false;
    }

    toString(strHex);

    strRet = "";
    for (size_t i = 0; i < strHex.size(); i++)
    {
        strRet += tfm::format("%02x", ((int)strHex[i]));
    }
    strOut = strRet;

    return true;
}

bool Check(const unsigned char *vch)
{
    auto *ctx = GetSecp256k1Ctx();
    return secp256k1_ec_seckey_verify(ctx, vch);
}

void htdf::MakeNewKey(unsigned char *key32)
{
    do
    {
        GetRandom32Bytes(key32);
    } while (!Check(key32));
}

string htdf::PubkToAddress(const string &strHexPubk)
{
    CHash160 hash160;
    vector<unsigned char> pubk = ParseHex(strHexPubk);
    vector<unsigned char> out(CRIPEMD160::OUTPUT_SIZE);
    hash160.Write(pubk);
    hash160.Finalize(out);

    vector<unsigned char> conv;
    bech32::convertbits<8, 5, true>(conv, out);
    string strBech32Addr = bech32::encode(STR_HTDF, conv);
    return strBech32Addr;
}