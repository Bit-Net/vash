#include <QApplication>
#include <QClipboard>
#include <string>
#include <vector>

#include "base58.h"
#include "multisiginputentry.h"
#include "ui_multisiginputentry.h"
#include "main.h"
#include "script.h"
#include "util.h"
#include "wallet.h"
#include "init.h"
#include "walletmodel.h"


MultisigInputEntry::MultisigInputEntry(QWidget *parent) : QFrame(parent), ui(new Ui::MultisigInputEntry), model(0)
{
    ui->setupUi(this);
}

MultisigInputEntry::~MultisigInputEntry()
{
    delete ui;
}

void MultisigInputEntry::setModel(WalletModel *model)
{
    this->model = model;
    clear();
    //on_listUnSpentTransactionsButton_clicked();
//ui->transactionId->lineEdit()
    ui->transactionId->lineEdit()->setPlaceholderText(tr("Enter a transaction id"));
    //connect(ui->transactionId, SIGNAL( textChanged(const QString &) ), this, SLOT( on_transactionId_textChanged(QString) ));
    connect(ui->transactionId->lineEdit(), SIGNAL( textChanged(const QString &) ), this, SLOT( on_transactionIdComb_textChanged(QString) ));
}

int listUnSpentTxs(std::vector<std::string> &txs)
{
    int rzt = 0, nMinDepth = 1, nMaxDepth = 9999999;
    vector<COutput> vecOutputs;
    pwalletMain->AvailableCoins(vecOutputs, false);
    BOOST_FOREACH(const COutput& out, vecOutputs)
    {
        if (out.nDepth < nMinDepth || out.nDepth > nMaxDepth)
            continue;

        /*if(setAddress.size())
        {
            CTxDestination address;   txnouttype txotp;
            if(!ExtractDestination(out.tx->vout[out.i].scriptPubKey, address, &txotp))
                continue;
            if( txotp != TX_SCRIPTHASH ){ continue; }
            if (!setAddress.count(address))
                continue;
        }*/

        const CScript& pk = out.tx->vout[out.i].scriptPubKey;
        CTxDestination address;   txnouttype txotp;
        if(!ExtractDestination(pk, address, &txotp)){ continue; }
        if( txotp != TX_SCRIPTHASH ){ continue; }
        txs.push_back(out.tx->GetHash().GetHex());   rzt++;
        /*int64_t nValue = out.tx->vout[out.i].nValue;
        Object entry;
        entry.push_back(Pair("txid", out.tx->GetHash().GetHex()));
        entry.push_back(Pair("vout", out.i));
        CTxDestination address;
        if (ExtractDestination(out.tx->vout[out.i].scriptPubKey, address))
        {
            entry.push_back(Pair("address", CBitcoinAddress(address).ToString()));
            if (pwalletMain->mapAddressBook.count(address))
                entry.push_back(Pair("account", pwalletMain->mapAddressBook[address]));
        }
        entry.push_back(Pair("scriptPubKey", HexStr(pk.begin(), pk.end())));
        if (pk.IsPayToScriptHash())
        {
            CTxDestination address;
            if (ExtractDestination(pk, address))
            {
                const CScriptID& hash = boost::get<CScriptID>(address);
                CScript redeemScript;
                if (pwalletMain->GetCScript(hash, redeemScript))
                    entry.push_back(Pair("redeemScript", HexStr(redeemScript.begin(), redeemScript.end())));
            }
        }
        entry.push_back(Pair("amount",ValueFromAmount(nValue)));
        entry.push_back(Pair("confirmations",out.nDepth));
        results.push_back(entry); */
    }
    return rzt;
}

void MultisigInputEntry::on_listUnSpentTransactionsButton_clicked()
{
    ui->transactionId->clear();
    std::vector<std::string> txs;
    std::vector<std::string>::iterator theIterator;
	int i = listUnSpentTxs(txs);
	if( i > 0 )
	{
        for( theIterator = txs.begin(); theIterator != txs.end(); theIterator++ )
		{
			QString q = QString::fromStdString(theIterator->c_str());
			ui->transactionId->addItem( q );
		}
	}
}

void MultisigInputEntry::clear()
{
    curTransactionId.clear();
    ui->transactionId->clear();
    ui->transactionOutput->clear();
    ui->redeemScript->clear();
}

bool MultisigInputEntry::validate()
{
    return (ui->transactionOutput->count() > 0);
}

CTxIn MultisigInputEntry::getInput()
{
    unsigned int nOutput = ui->transactionOutput->currentIndex();
    CTxIn input(COutPoint(txHash, nOutput));

    return input;
}

int64_t MultisigInputEntry::getAmount()
{
    int64_t amount = 0;
    unsigned int nOutput = ui->transactionOutput->currentIndex();
    CTransaction tx;
    uint256 blockHash = 0;

    if(GetTransaction(txHash, tx, blockHash))
    {
        if(nOutput < tx.vout.size())
        {
            const CTxOut& txOut = tx.vout[nOutput];
            amount = txOut.nValue;
        }
    }

    return amount;
}

QString MultisigInputEntry::getRedeemScript()
{
    return ui->redeemScript->text();
}

void MultisigInputEntry::setTransactionId(QString transactionId)
{
    //ui->transactionId->setText(transactionId);
    ui->transactionId->lineEdit()->setText(transactionId);
}

void MultisigInputEntry::setTransactionOutputIndex(int index)
{
    ui->transactionOutput->setCurrentIndex(index);
}

void MultisigInputEntry::setRemoveEnabled(bool enabled)
{
    ui->deleteButton->setEnabled(enabled);
}

void MultisigInputEntry::on_pasteTransactionIdButton_clicked()
{
    //ui->transactionId->setText(QApplication::clipboard()->text());
    ui->transactionId->lineEdit()->setText(QApplication::clipboard()->text());
    //on_transactionId_textChanged
}

void MultisigInputEntry::on_deleteButton_clicked()
{
    emit removeEntry(this);
}

void MultisigInputEntry::on_pasteRedeemScriptButton_clicked()
{
    ui->redeemScript->setText(QApplication::clipboard()->text());
}

void MultisigInputEntry::on_transactionIdComb_textChanged(const QString &transactionId)
{
    if( curTransactionId != transactionId )
	{
		onTransactionIdChanged(transactionId);   curTransactionId = transactionId;
	}
}

//extern void ShowModMsgBox(QString qTitle, QString qText);
void MultisigInputEntry::on_transactionId_textChanged(const QString &transactionId)
{

}

void MultisigInputEntry::onTransactionIdChanged(const QString &transactionId)
{
    ui->transactionOutput->clear();   // transactionId
//ShowModMsgBox("onTransactionIdChanged 001", transactionId);
//printf("onTransactionIdChanged 001 [%s] \n", transactionId.toStdString().c_str());
    if(transactionId.isEmpty())
        return;
//ShowModMsgBox("onTransactionIdChanged 002", transactionId);
//printf("onTransactionIdChanged 002 [%s] \n", transactionId.toStdString().c_str());

    // Make list of transaction outputs
    txHash.SetHex(transactionId.toStdString().c_str());
    CTransaction tx;
    uint256 blockHash = 0;
    if(!GetTransaction(txHash, tx, blockHash))
        return;
    bool bSeted = false;
    for(unsigned int i = 0; i < tx.vout.size(); i++)
    {
        QString idStr;
        idStr.setNum(i);
        const CTxOut& txOut = tx.vout[i];
        int64_t amount = txOut.nValue;
        QString amountStr;
        amountStr.sprintf("%.6f", (double) amount / COIN);
        CScript script = txOut.scriptPubKey;
        CTxDestination addr;   txnouttype txotp;
        //if(!ExtractDestination(script, addr, &txotp)){ continue; }
        //if( txotp != TX_SCRIPTHASH ){ continue; }
        if(ExtractDestination(script, addr, &txotp))
        {
            CBitcoinAddress address(addr);
            QString addressStr(address.ToString().c_str());
            ui->transactionOutput->addItem(idStr + QString(" - ") + addressStr + QString(" - ") + amountStr + QString(" VPN"));
			if( !bSeted )
			{
			    if( txotp == TX_SCRIPTHASH ){ ui->transactionOutput->setCurrentIndex(i);   bSeted = true; }
			}
        }
        else
            ui->transactionOutput->addItem(idStr + QString(" - ") + amountStr + QString(" VPN"));
    }
}

void MultisigInputEntry::on_transactionOutput_currentIndexChanged(int index)
{
    ui->redeemScript->clear();
    if(ui->transactionOutput->itemText(index).isEmpty())
        return;

    CTransaction tx;
    uint256 blockHash = 0;
    if(!GetTransaction(txHash, tx, blockHash))
        return;
    const CTxOut& txOut = tx.vout[index];
    CScript script = txOut.scriptPubKey;

    if(script.IsPayToScriptHash())
    {
        ui->redeemScript->setEnabled(true);

        if(model)
        {
             // Try to find the redeem script
             CTxDestination dest;
            if(ExtractDestination(script, dest))
            {
                CScriptID scriptID = boost::get<CScriptID>(dest);
                CScript redeemScript;
                if(model->getWallet()->GetCScript(scriptID, redeemScript))
                    ui->redeemScript->setText(HexStr(redeemScript.begin(), redeemScript.end()).c_str());
            }
        }
    }
    else
    {
        ui->redeemScript->setEnabled(false);
    }

    emit updateAmount();
}