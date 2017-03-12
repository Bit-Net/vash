#include "transactionrecord.h"

#include "wallet.h"
#include "base58.h"

#include "bitcoinunits.h"

#ifdef USE_BITNET
#include "bitnet.h"
#endif

#include <string>
#include "simplecrypt.h"
#include "zkpdialog.h"

using namespace std;
using namespace boost;

/* Return positive answer if transaction should be shown in list.
 */
bool TransactionRecord::showTransaction(const CWalletTx &wtx)
{
    if (wtx.IsCoinBase())
    {
        // Ensures we show generated coins / mined transactions at depth 1
        if (!wtx.IsInMainChain())
        {
            return false;
        }
    }
    return true;
}

/*
 * Decompose CWallet transaction to model transaction records.
 */
 extern std::string BitChain_Head;
 extern ZkpForm* gZkpForm;
QList<TransactionRecord> TransactionRecord::decomposeTransaction(const CWallet *wallet, const CWalletTx &wtx)
{
    QList<TransactionRecord> parts;
    int64_t nTime = wtx.GetTxTime();
    int64_t nCredit = wtx.GetCredit(true);
    int64_t nDebit = wtx.GetDebit();
    int64_t nNet = nCredit - nDebit;
    uint256 hash = wtx.GetHash(), hashPrev = 0;
    std::map<std::string, std::string> mapValue = wtx.mapValue;

    std::string txcomment = "";     bool bZkpBurnTx=false, bZkpMintTx=false;
	std::string stxData = wtx.vpndata.c_str();
    if (!stxData.empty())
    {
        bool b = true;
		if( gZkpForm ){
			QString qRztZkpHash="";
			if( gZkpForm->expZkpBurnTx((CTransaction&)wtx, true, qRztZkpHash) )
			{
				bZkpBurnTx=true;     b=false;   txcomment = qRztZkpHash.toStdString();
			}
			if( b ){
				if( gZkpForm->expZkpMintTx((CTransaction&)wtx, qRztZkpHash) )
				{
					bZkpMintTx=true;     b=false;   txcomment = qRztZkpHash.toStdString();
				}
			}
		}
		if( b )
		{
			if( (stxData.find(BitChain_Head) != std::string::npos) || (stxData.find("@FROM=") != std::string::npos) || (stxData.find("@SUBJ=") != std::string::npos) || (stxData.find("@MSG=") != std::string::npos) || (stxData.find("BitNet") != std::string::npos) )
			{
				txcomment = stxData;
			}else{
				SimpleCrypt processSimpleCrypt((quint64)wtx.nTime);
				QString q = processSimpleCrypt.decryptToString(QString(stxData.c_str()));
				if( q.isEmpty() == false )
				{
					stxData = q.toStdString();
				}
				txcomment = stxData;
			}
		}
    }

    if (nNet > 0 || wtx.IsCoinBase() || wtx.IsCoinStake())
    {
        //
        // Credit
        //
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            if(wallet->IsMine(txout))
            {
                TransactionRecord sub(hash, nTime);
                CTxDestination address;
                sub.txcomment = txcomment;
                sub.idx = parts.size(); // sequence number
                sub.credit = txout.nValue;
                if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*wallet, address))
                {
                    // Received by Bitcoin Address
                    if( !bZkpMintTx ){ sub.type = TransactionRecord::RecvWithAddress; }
					else{ sub.type = TransactionRecord::MintCoin; }
                    sub.address = CBitcoinAddress(address).ToString();
                }
                else
                {
                    // Received by IP connection (deprecated features), or a multisignature or other non-simple transaction
                    sub.type = TransactionRecord::RecvFromOther;
                    sub.address = mapValue["from"];
                }
                if (wtx.IsCoinBase())
                {
                    // Generated (proof-of-work)
                    sub.type = TransactionRecord::Generated;
                }
                if (wtx.IsCoinStake())
                {
                    // Generated (proof-of-stake)

                    if (hashPrev == hash)
                        continue; // last coinstake output

                    sub.type = TransactionRecord::Generated;
                    sub.credit = nNet > 0 ? nNet : wtx.GetValueOut() - nDebit;
                    hashPrev = hash;
                }

                parts.append(sub);
            }
        }
    }
    else
    {
        bool fAllFromMe = true;
        BOOST_FOREACH(const CTxIn& txin, wtx.vin)
            fAllFromMe = fAllFromMe && wallet->IsMine(txin);

        bool fAllToMe = true;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            fAllToMe = fAllToMe && wallet->IsMine(txout);

        if (fAllFromMe && fAllToMe)
        {
            // Payment to self
            int64_t nChange = wtx.GetChange();

            parts.append(TransactionRecord(hash, nTime, TransactionRecord::SendToSelf, "",
                            -(nDebit - nChange), nCredit - nChange, txcomment));
        }
        else if (fAllFromMe)
        {
            //
            // Debit
            //
            int64_t nTxFee = nDebit - wtx.GetValueOut();

            for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++)
            {
                const CTxOut& txout = wtx.vout[nOut];
                TransactionRecord sub(hash, nTime);
                sub.idx = parts.size();
                sub.txcomment = txcomment;

                if(wallet->IsMine(txout))
                {
                    // Ignore parts sent to self, as this is usually the change
                    // from a transaction sent back to our own address.
                    continue;
                }

                CTxDestination address;
                if (ExtractDestination(txout.scriptPubKey, address))
                {
                    // Sent to Bitcoin Address
                    if( !bZkpBurnTx ){ sub.type = TransactionRecord::SendToAddress; }
					else{ sub.type = TransactionRecord::BurnCoin; }
                    sub.address = CBitcoinAddress(address).ToString();
                }
                else
                {
                    // Sent to IP, or other non-address transaction like OP_EVAL
                    sub.type = TransactionRecord::SendToOther;
                    sub.address = mapValue["to"];
                }

                int64_t nValue = txout.nValue;
                /* Add fee to first output */
                if (nTxFee > 0)
                {
                    nValue += nTxFee;
                    nTxFee = 0;
                }
                sub.debit = -nValue;

                parts.append(sub);
            }
        }
        else
        {
            //
            // Mixed debit transaction, can't break down payees
            //
            parts.append(TransactionRecord(hash, nTime, TransactionRecord::Other, "", nNet, 0, txcomment));
        }
    }

    return parts;
}

void TransactionRecord::updateStatus(const CWalletTx &wtx)
{
    AssertLockHeld(cs_main);
    // Determine transaction status

    // Find the block the tx is in
    CBlockIndex* pindex = NULL;
    std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(wtx.hashBlock);
    if (mi != mapBlockIndex.end())
        pindex = (*mi).second;

    // Sort order, unrecorded transactions sort to the top
    status.sortKey = strprintf("%010d-%01d-%010u-%03d",
        (pindex ? pindex->nHeight : std::numeric_limits<int>::max()),
        (wtx.IsCoinBase() ? 1 : 0),
        wtx.nTimeReceived,
        idx);
    status.countsForBalance = wtx.IsTrusted() && !(wtx.GetBlocksToMaturity() > 0);
    status.depth = wtx.GetDepthInMainChain();
    status.cur_num_blocks = nBestHeight;

	if (!IsFinalTx(wtx, nBestHeight + 1))
    {
        if (wtx.nLockTime < LOCKTIME_THRESHOLD)
        {
            status.status = TransactionStatus::OpenUntilBlock;
            status.open_for = wtx.nLockTime - nBestHeight;
        }
        else
        {
            status.status = TransactionStatus::OpenUntilDate;
            status.open_for = wtx.nLockTime;
        }
    }

    // For generated transactions, determine maturity
    else if(type == TransactionRecord::Generated)
    {
        if (wtx.GetBlocksToMaturity() > 0)
        {
            status.status = TransactionStatus::Immature;

            if (wtx.IsInMainChain())
            {
                status.matures_in = wtx.GetBlocksToMaturity();

                // Check if the block was requested by anyone
                if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                    status.status = TransactionStatus::MaturesWarning;
            }
            else
            {
                status.status = TransactionStatus::NotAccepted;
            }
        }
        else
        {
            status.status = TransactionStatus::Confirmed;
        }
    }
    else
    {
        if(type == TransactionRecord::BurnCoin)
        {
            if( gZkpForm )
            {
                QString qRztZkpHash="";
                if( gZkpForm->expZkpBurnTx((CTransaction&)wtx, true, qRztZkpHash) )
                {
                    std::string s = qRztZkpHash.toStdString();
                    if( s != txcomment ){ txcomment = s; }
                }
            }
		}
        if (status.depth < 0)
        {
            status.status = TransactionStatus::Conflicted;
        }
        else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
        {
            status.status = TransactionStatus::Offline;
        }
        else if (status.depth == 0)
        {
            status.status = TransactionStatus::Unconfirmed;
        }
        else if (status.depth < RecommendedNumConfirmations)
        {
            status.status = TransactionStatus::Confirming;
        }
        else
        {
            status.status = TransactionStatus::Confirmed;
        }
		
#ifdef USE_BITNET
		if( GetArg("-synctx2gui", 1) )
		if( (this->type == TransactionRecord::RecvWithAddress) || (this->type == TransactionRecord::RecvFromOther) )
		{

			//printf("TransactionRecord::updateStatus --->>> type=%u, nHeight=%u, depth=%u, %I64u + %I64u\n", type, dHei, status.depth, credit, debit);
			//string str;	//double value = atof(str.c_str());
	
			//catch (std::exception & e){ PrintExceptionContinue(&e, "TransactionRecord::updateStatus()"); }
			//catch (...){ PrintExceptionContinue(NULL, "TransactionRecord::updateStatus()"); }
	
			/* std::string stxData = wtx.vpndata;
			if( !stxData.length() ){ stxData = "Test_AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaa"; }
			int64_t txTime = wtx.nTime;
			//DWORD SyncTransactionToGui(std::string& txId, std::string& txMsg, DWORD iDepth, DWORD nHeight, DWORD curHeight, int64_t TxAmount, int64_t txTime)
			SyncTransactionToGui(sTranId, stxData, status.depth, pindex->nHeight, status.cur_num_blocks, iAmount, txTime);
			*/
			try{
                //string str;
                //double value = atof(str.c_str());
				//if( dStartVpnServer )
			
				DWORD dHei = 0;
				if( pindex ){ dHei = pindex->nHeight;}
				qint64 iAmount = credit + debit;
				QString qStr = BitcoinUnits::format(BitcoinUnits::BTC, iAmount);
				std::string str = qStr.toStdString();
				std::string sTranId = wtx.GetHash().ToString();
				//printf("TransactionTableModel::data->Type:: %u, dep=%u, %s %s\n", this->type, this->status.depth, str.toStdString().c_str(), sAmount.c_str());
				/* if( fDebug )
				{
					printf("\n");
					printf("TransactionRecord::updateStatus: type=%d, nHeight=%u, BestHeight=%u, depth=%u\n", this->type, dHei, nBestHeight, status.depth);
					printf("TransactionRecord::updateStatus: %I64u, amt=%s, txid=%s \n", iAmount, str.c_str(), sTranId.c_str());
				} */
                    std::string stxData = wtx.vpndata;
                    int64_t txTime = wtx.nTime;

                    if( stxData.length() > 19 )
                    {
                        int iOk = 0;
                        if( (stxData.find("@FROM=") != std::string::npos || stxData.find("@SUBJ=") != std::string::npos) && (stxData.find("@MSG=") != std::string::npos) )
                        {
                           iOk++;
                        }
                        else
                        {
                            SimpleCrypt processSimpleCrypt((quint64)wtx.nTime);
                            //stxData.clear();
                            stxData = processSimpleCrypt.decryptToString(QString(stxData.c_str())).toStdString();
                            if( (stxData.find("@FROM=") != std::string::npos || stxData.find("@SUBJ=") != std::string::npos) && (stxData.find("@MSG=") != std::string::npos) )
                                iOk++;
                        }
                        //if( fDebug ){ printf("TransactionTableModel::wtx->vpndata:: %s, nTime=%I64u\n", stxData.c_str(), txTime); }
                        if( iOk ){
                            //std::string sTranId = wtx.GetHash().ToString();  //Transaction ID
                        //DWORD SyncTransactionToGui(std::string& txId, std::string& txMsg, DWORD iDepth, DWORD nHeight, DWORD curHeight, int64_t TxAmount, int64_t txTime)
                            SyncTransactionToGui(sTranId, stxData, status.depth, dHei, status.cur_num_blocks, iAmount, txTime);
                        }
                    }
				//printf("\n");
            }catch (std::exception & e)
            {
                // do something with what...
            }
            catch (...)
            {
                // someone threw something undecypherable
            }
		}
#endif
    }
}

bool TransactionRecord::statusUpdateNeeded()
{
    AssertLockHeld(cs_main);
    return status.cur_num_blocks != nBestHeight;
}

std::string TransactionRecord::getTxID()
{
    return hash.ToString() + strprintf("-%03d", idx);
}

