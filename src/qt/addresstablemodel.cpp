#include "addresstablemodel.h"

#include "guiutil.h"
#include "walletmodel.h"
#include "util.h"
#include "wallet.h"
//#include "rpcwallet.h"
#include "base58.h"

#include <QFont>
using namespace std;

const QString AddressTableModel::Send = "S";
const QString AddressTableModel::Receive = "R";

struct AddressTableEntry
{
    enum Type {
        Sending,
        Receiving
    };

    Type type;
    QString label;
    QString address;
	bool IsDefault;
	bool IsMine;
	int64_t Balance;

    AddressTableEntry() {}
    AddressTableEntry(int64_t Balance, bool IsMine, bool IsDefault, Type type, const QString &label, const QString &address):
        Balance(Balance), IsMine(IsMine), IsDefault(IsDefault), type(type), label(label), address(address) {}
};

struct AddressTableEntryLessThan
{
    bool operator()(const AddressTableEntry &a, const AddressTableEntry &b) const
    {
        return a.address < b.address;
    }
    bool operator()(const AddressTableEntry &a, const QString &b) const
    {
        return a.address < b;
    }
    bool operator()(const QString &a, const AddressTableEntry &b) const
    {
        return a < b.address;
    }
};

// Private implementation
double nValueFromAmount(int64_t amount)
{
    return (double)amount / (double)COIN;
}

int iShowBalance = 0;
extern int iHideAddress;
//extern AddressBookPage *receiveCoinsPage;
//QTableView *rcvTable = NULL;
extern void setTabView(QTableView *view, int iRow, int iView);
extern int64_t GetAccountBalance(const std::string& strAccount, int nMinDepth = 1);

bool isBitNetAddress(string sLab)
{
	bool rzt = false;
	int i = sLab.find("BitNetCheque");
	int j = sLab.find("BitNetLottery");
	int k = sLab.find("BitNetAdBonus");
	if( (i == 0) || (i == 1) || (j == 0) || (j == 1) || (k == 0) || (k == 1) ){ rzt = true; }	
	return rzt;
}

class AddressTablePriv
{
public:
    CWallet *wallet;
    QList<AddressTableEntry> cachedAddressTable;
    AddressTableModel *parent;

    AddressTablePriv(CWallet *wallet, AddressTableModel *parent):
        wallet(wallet), parent(parent) {}

    void refreshAddressTable(int iOpt)
    {
        if( iOpt == 0 ){ cachedAddressTable.clear(); }
        {
            //LOCK(wallet->cs_wallet);
            std::string sDef = CBitcoinAddress(wallet->vchDefaultKey.GetID()).ToString();
			LOCK(wallet->cs_wallet);
            BOOST_FOREACH(const PAIRTYPE(CTxDestination, std::string)& item, wallet->mapAddressBook)
            {
                const CBitcoinAddress& address = item.first;
                const std::string& strName = item.second;
				string sAddr = address.ToString();
                bool fMine = IsMine(*wallet, address.Get());			
				bool IsDef = false;
				int64_t iBalance = 0;
				if( fMine && (sDef.length() == 34) )
				{
					if( sDef.find(address.ToString()) == 0 ){ IsDef = true; }
					if( (iShowBalance > 0) && (strName.length()) )
					{
						iBalance = GetAccountBalance(strName, 0);
					}
				}
				
				if( (iHideAddress > 0) && fMine )
				{
					if( isBitNetAddress(strName) )
					{
						continue;
					}
				}
				if( iOpt == 0 )
				{
					//CBitcoinAddress(pwalletMain->vchDefaultKey.GetID()).ToString();
					cachedAddressTable.append(AddressTableEntry(iBalance, fMine, IsDef, fMine ? AddressTableEntry::Receiving : AddressTableEntry::Sending,
                                  QString::fromStdString(strName),
                                  QString::fromStdString(address.ToString())));
				}
				
				/*if( iOpt > 0 )
				{
				//int i = GetArg("-hidebntaddr", 1);  
				QString qAddr = QString::fromStdString(sAddr);
				int iRow = parent->lookupAddress(qAddr);
				if( fDebug ){ printf("refreshAddressTable iHide = [%d],  Row [%d] [%s] [%s]\n", iHideAddress, iRow, strName.c_str(), sAddr.c_str()); }
				if( fMine && ( iRow >= 0 ) ) 
				{
					if( (rcvTable != NULL) && isBitNetAddress(strName) )
					{ 
						if( fDebug ){ printf("refreshAddressTable iHide = [%d],  [%s] [%s]\n", iHideAddress, strName.c_str(), sAddr.c_str()); }
						if( iHideAddress > 0 )
						{
							setTabView(rcvTable, iRow, 0); //rcvTable->hideRow(iRow);
						}
						else setTabView(rcvTable, iRow, 1);  //rcvTable->showRow( iRow );
						//continue; 
					}		
				}
				}*/
				/*if( iOpt > 0 ){
					QString qLab = QString::fromStdString(strName);
					parent->updateEntry(qAddr, qLab, fMine, CT_UPDATED);
				}*/
            }
        }
        // qLowerBound() and qUpperBound() require our cachedAddressTable list to be sorted in asc order
        if( iOpt == 0 ){ qSort(cachedAddressTable.begin(), cachedAddressTable.end(), AddressTableEntryLessThan()); }
    }

    void updateEntry(const QString &address, const QString &label, bool isMine, int status)
    {
        // Find address / label in model
        QList<AddressTableEntry>::iterator lower = qLowerBound(
            cachedAddressTable.begin(), cachedAddressTable.end(), address, AddressTableEntryLessThan());
        QList<AddressTableEntry>::iterator upper = qUpperBound(
            cachedAddressTable.begin(), cachedAddressTable.end(), address, AddressTableEntryLessThan());
        int lowerIndex = (lower - cachedAddressTable.begin());
        int upperIndex = (upper - cachedAddressTable.begin());
        bool inModel = (lower != upper);
        AddressTableEntry::Type newEntryType = isMine ? AddressTableEntry::Receiving : AddressTableEntry::Sending;

		if( fDebug ){ printf("updateEntry [%d] [%s] [%s]\n", status, label.toStdString().c_str(), address.toStdString().c_str()); }
		
		bool IsDef = false;	
		int64_t iBalance = 0;
		if( isMine )
		{
			std::string sDef = CBitcoinAddress(wallet->vchDefaultKey.GetID()).ToString();
			if( sDef.length() == 34 )
			{
				if( sDef.find(address.toStdString()) == 0 ){ IsDef = true; }
			}
			std::string sLab = label.toStdString();
			if( (iShowBalance > 0) && (sLab.length()) )
			{
				iBalance = GetAccountBalance(sLab, 0);
			}
		}
		
        switch(status)
        {
        case CT_NEW:
            if(inModel)
            {
                OutputDebugStringF("Warning: AddressTablePriv::updateEntry: Got CT_NOW, but entry is already in model\n");
                break;
            }
            parent->beginInsertRows(QModelIndex(), lowerIndex, lowerIndex);
					
            cachedAddressTable.insert(lowerIndex, AddressTableEntry(iBalance, isMine, IsDef, newEntryType, label, address));
            parent->endInsertRows();
            break;
        case CT_UPDATED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: AddressTablePriv::updateEntry: Got CT_UPDATED, but entry is not in model\n");
                break;
            }
            lower->type = newEntryType;
            lower->label = label;
			if( isMine )
			{
				lower->IsDefault = IsDef;
				lower->Balance   = iBalance;
				lower->IsMine    = isMine;
			}
            parent->emitDataChanged(lowerIndex);
            break;
        case CT_DELETED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: AddressTablePriv::updateEntry: Got CT_DELETED, but entry is not in model\n");
                break;
            }
            parent->beginRemoveRows(QModelIndex(), lowerIndex, upperIndex-1);
            cachedAddressTable.erase(lower, upper);
            parent->endRemoveRows();
            break;
        }
    }

    int size()
    {
        return cachedAddressTable.size();
    }

    AddressTableEntry *index(int idx)
    {
        if(idx >= 0 && idx < cachedAddressTable.size())
        {
            return &cachedAddressTable[idx];
        }
        else
        {
            return 0;
        }
    }
};

AddressTableModel::AddressTableModel(CWallet *wallet, WalletModel *parent) :
    QAbstractTableModel(parent),walletModel(parent),wallet(wallet),priv(0)
{
    columns << tr("Label") << tr("Address") << tr("Default");	// << tr("Balance");
	iShowBalance = GetArg("-showrecvbalance", 0);
	if( iShowBalance ){ columns << tr("Balance"); }
    priv = new AddressTablePriv(wallet, this);
    priv->refreshAddressTable(0);
}

AddressTableModel::~AddressTableModel()
{
    delete priv;
}

int AddressTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return priv->size();
}

int AddressTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columns.length();
}

std::string dtos(double x) {
    std::stringstream s;  // Allocates memory on stack
    s << x;
    return s.str();       // returns a s.str() as a string by value
                          // Frees allocated memory of s
}

QVariant AddressTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    AddressTableEntry *rec = static_cast<AddressTableEntry*>(index.internalPointer());

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Label:
            if(rec->label.isEmpty() && role == Qt::DisplayRole)
            {
                return tr("(no label)");
            }
            else
            {
                return rec->label;
            }
        case Address:
            return rec->address;
		case 2:	//IsDefault
			if( rec->IsDefault ) return tr("Yes");
			else return tr("No");
		case 3: // Balance
			if( rec->IsMine )
			{
				//if( rec->Balance )	//if( rec->IsMine )
				{
					double d = nValueFromAmount(rec->Balance);
					std::string s = strprintf("%f", d); //dtos(d);
					if( fDebug ){ printf("[%s] d = [%I64u] [%f]\n", s.c_str(), rec->Balance, d); }
					QString sq = QString::fromStdString(s);   //QString s = QString::number(d);
					return sq;
				}
				/*else{
					if( rec->label.isEmpty() )
					{
						return tr("(no label)");
					}
					return "";
				}*/
			}else{ return ""; }
		}
    }
    else if (role == Qt::FontRole)
    {
        QFont font;
        if(index.column() == Address)
        {
			font = GUIUtil::bitcoinAddressFont();
        }
        if( rec->IsDefault )
		{
			font.setBold(true);
		}
		/* else if( (index.column() == Label) && (!rec->label.isEmpty()) )
		{
			std::string sLab = rec->label.toStdString();
			int i = sLab.find("BitNetCheque");
			if( (i == 0) || (i == 1) ){ font.setStrikeOut(true); }
		} */
        return font;
    }
	else if (role == Qt::ForegroundRole )
	{
		QColor color = Qt::black;
		if( (index.column() == Label) && (!rec->label.isEmpty()) )
		{
			std::string sLab = rec->label.toStdString();
			//int i = sLab.find("BitNetCheque");
			//int j = sLab.find("BitNetLottery");
			//int k = sLab.find("BitNetAdBonus");
			if( isBitNetAddress(sLab) )  //if( (i == 0) || (i == 1) || (j == 0) || (j == 1) || (k == 0) || (k == 1) )
			{
				color = Qt::red;
			}		
		
		/*if( rec->IsDefault )	//if( (rec->type == AddressTableEntry::Receiving) && rec->IsDefault )
		{
			color = Qt::green;
		}*/
		}
		return color;
	}
	/*else if (role == Qt::BackgroundRole )
	{
		QColor color = Qt::white;
		
		if( !rec->label.isEmpty() )
		{
		std::string sLab = rec->label.toStdString();
		int i = sLab.find("BitNetCheque");
		if( (i == 0) || (i == 1) )
		{
			color = Qt::red;
		}}		
		
		if( rec->IsDefault )	//if( (rec->type == AddressTableEntry::Receiving) && rec->IsDefault )
		{
			color = Qt::green;
		}
		color.setAlpha(50); 
		QBrush redBackground(color);	//QBrush redBackground(Qt::green);
		return redBackground;
	} */
    else if (role == TypeRole)
    {
        switch(rec->type)
        {
        case AddressTableEntry::Sending:
            return Send;
        case AddressTableEntry::Receiving:
            return Receive;
        default: break;
        }
    }
    return QVariant();
}

bool AddressTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;
    AddressTableEntry *rec = static_cast<AddressTableEntry*>(index.internalPointer());

    editStatus = OK;

    if(role == Qt::DisplayRole || role == Qt::EditRole)	//if(role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Label:
			// Do nothing, if old label == new label
            if(rec->label == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
            wallet->SetAddressBookName(CBitcoinAddress(rec->address.toStdString()).Get(), value.toString().toStdString());
            break;
        case Address:
            // Do nothing, if old address == new address
			if(CBitcoinAddress(rec->address.toStdString()) == CBitcoinAddress(value.toString().toStdString()))
            {
                editStatus = NO_CHANGES;
                return false;
            }
            // Refuse to set invalid address, set error status and return false
            else if(!walletModel->validateAddress(value.toString()))
            {
                editStatus = INVALID_ADDRESS;
                return false;
            }
            // Check for duplicate addresses to prevent accidental deletion of addresses, if you try
            // to paste an existing address over another address (with a different label)
            else if(wallet->mapAddressBook.count(CBitcoinAddress(value.toString().toStdString()).Get()))
            {
                editStatus = DUPLICATE_ADDRESS;
                return false;
            }
            // Double-check that we're not overwriting a receiving address
            else if(rec->type == AddressTableEntry::Sending)
            {
                {
                    LOCK(wallet->cs_wallet);
                    // Remove old entry
                    wallet->DelAddressBookName(CBitcoinAddress(rec->address.toStdString()).Get());
                    // Add new entry with new address
                    wallet->SetAddressBookName(CBitcoinAddress(value.toString().toStdString()).Get(), rec->label.toStdString());
                }
            }
            break;
        }
        return true;
    }
    return false;
}

QVariant AddressTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return columns[section];
        }
    }
    return QVariant();
}

Qt::ItemFlags AddressTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return 0;
    AddressTableEntry *rec = static_cast<AddressTableEntry*>(index.internalPointer());

    Qt::ItemFlags retval = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    // Can edit address and label for sending addresses,
    // and only label for receiving addresses.
    if(rec->type == AddressTableEntry::Sending ||
      (rec->type == AddressTableEntry::Receiving && index.column()==Label))
    {
        retval |= Qt::ItemIsEditable;
    }
    return retval;
}

QModelIndex AddressTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    AddressTableEntry *data = priv->index(row);
    if(data)
    {
        return createIndex(row, column, priv->index(row));
    }
    else
    {
        return QModelIndex();
    }
}

void AddressTableModel::updateEntry(const QString &address, const QString &label, bool isMine, int status)
{
    // Update address book model from Bitcoin core
	//OutputDebugStringF("AddressTableModel::updateEntry %s %s isMine=%i status=%i\n", address->ToStdString().c_str(), label->toStdString.c_str(), isMine, status);
    priv->updateEntry(address, label, isMine, status);
}

QString AddressTableModel::addRow(const QString &type, const QString &label, const QString &address)
{
    std::string strLabel = label.toStdString();
    std::string strAddress = address.toStdString();

    editStatus = OK;

    if(type == Send)
    {
        if(!walletModel->validateAddress(address))
        {
            editStatus = INVALID_ADDRESS;
            return QString();
        }
        // Check for duplicate addresses
        {
            LOCK(wallet->cs_wallet);
            if(wallet->mapAddressBook.count(CBitcoinAddress(strAddress).Get()))
            {
                editStatus = DUPLICATE_ADDRESS;
                return QString();
            }
        }
    }
    else if(type == Receive)
    {
        // Generate a new address to associate with given label
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());
        if(!ctx.isValid())
        {
            // Unlock wallet failed or was cancelled
            editStatus = WALLET_UNLOCK_FAILURE;
            return QString();
        }
        CPubKey newKey;
        if(!wallet->GetKeyFromPool(newKey, true))
        {
            editStatus = KEY_GENERATION_FAILURE;
            return QString();
        }
        strAddress = CBitcoinAddress(newKey.GetID()).ToString();
    }
    else
    {
        return QString();
    }

    // Add entry
    {
        LOCK(wallet->cs_wallet);
        wallet->SetAddressBookName(CBitcoinAddress(strAddress).Get(), strLabel);
    }
    return QString::fromStdString(strAddress);
}

bool AddressTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    AddressTableEntry *rec = priv->index(row);
    if(count != 1 || !rec || rec->type == AddressTableEntry::Receiving)
    {
        // Can only remove one row at a time, and cannot remove rows not in model.
        // Also refuse to remove receiving addresses.
        return false;
    }
    {
        LOCK(wallet->cs_wallet);
        wallet->DelAddressBookName(CBitcoinAddress(rec->address.toStdString()).Get());
    }
    return true;
}

/* Look up label for address in address book, if not found return empty string.
 */
QString AddressTableModel::labelForAddress(const QString &address) const
{
    {
        LOCK(wallet->cs_wallet);
        CBitcoinAddress address_parsed(address.toStdString());
        std::map<CTxDestination, std::string>::iterator mi = wallet->mapAddressBook.find(address_parsed.Get());
        if (mi != wallet->mapAddressBook.end())
        {
            return QString::fromStdString(mi->second);
        }
    }
    return QString();
}

int AddressTableModel::lookupAddress(const QString &address) const
{
    QModelIndexList lst = match(index(0, Address, QModelIndex()),
                                Qt::EditRole, address, 1, Qt::MatchExactly);
    if(lst.isEmpty())
    {
        return -1;
    }
    else
    {
        return lst.at(0).row();
    }
}

void AddressTableModel::RefreshAddressTable(int iOpt)
{
	if( priv )
		priv->refreshAddressTable(iOpt);
}

void AddressTableModel::Clear2()
{
	//reset();
	beginResetModel();
	endResetModel();
}

void AddressTableModel::emitDataChanged(int idx)
{
    emit dataChanged(index(idx, 0, QModelIndex()), index(idx, columns.length()-1, QModelIndex()));
}
