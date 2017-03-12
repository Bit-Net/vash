#include "addressbookpage.h"
#include "ui_addressbookpage.h"

#include "addresstablemodel.h"
#include "optionsmodel.h"
#include "bitcoingui.h"
#include "editaddressdialog.h"
#include "csvmodelwriter.h"
#include "guiutil.h"
#include "util.h"
#include "base58.h"
#include "wallet.h"
#include "init.h"

#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>

#ifdef USE_QRCODE
#include "qrcodedialog.h"
#endif

using namespace std;
int iHideAddress = 0;
int iCanSet_HideAddress = 0;
//extern QTableView *rcvTable;
extern string dumpPrivaKey(string strAddress);
extern bool importPrivkey( const string strSecret, const string strLabel );

AddressBookPage::AddressBookPage(Mode mode, Tabs tab, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddressBookPage),
    model(0),
    optionsModel(0),
    mode(mode),
    tab(tab)
{
    ui->setupUi(this);
	iHideAddress = GetArg("-hidebntaddress", 1);
	//if( fDebug ){ printf("AddressBookPage iHideAddress = %d\n", iHideAddress); }

#ifdef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    ui->newAddressButton->setIcon(QIcon());
    ui->copyToClipboard->setIcon(QIcon());
    ui->deleteButton->setIcon(QIcon());
#endif

#ifndef USE_QRCODE
    ui->showQRCode->setVisible(false);
#endif

    switch(mode)
    {
    case ForSending:
        connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView->setFocus();
        break;
    case ForEditing:
        ui->buttonBox->setVisible(false);
        break;
    }
    switch(tab)
    {
    case SendingTab:
        ui->labelExplanation->setVisible(false);
        ui->deleteButton->setVisible(true);
        ui->signMessage->setVisible(false);
        ui->exportKeyButton->setVisible(false);
		ui->cbHideAddress->setVisible(false);
        break;
    case ReceivingTab:
        ui->deleteButton->setVisible(false);
        ui->signMessage->setVisible(true);
        break;
    }

    // Context menu actions
    QAction *copyLabelAction = new QAction(tr("Copy &Label"), this);
    QAction *copyAddressAction = new QAction(ui->copyToClipboard->text(), this);
    QAction *editAction = new QAction(tr("&Edit"), this);
    QAction *showInBlockExplorerAction = new QAction(tr("Show address in blockchain explorer"), this);
    QAction *showQRCodeAction = new QAction(ui->showQRCode->text(), this);
    QAction *signMessageAction = new QAction(ui->signMessage->text(), this);
    QAction *verifyMessageAction = new QAction(ui->verifyMessage->text(), this);
    deleteAction = new QAction(ui->deleteButton->text(), this);
    QAction *copyPubKeyAction = new QAction(tr("Copy Public Key"), this);

    // Build context menu
    contextMenu = new QMenu();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyLabelAction);
    if(tab == ReceivingTab){ contextMenu->addAction(copyPubKeyAction); }
    contextMenu->addAction(editAction);
    contextMenu->addAction(showInBlockExplorerAction);
    if(tab == SendingTab)
        contextMenu->addAction(deleteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(showQRCodeAction);
    if(tab == ReceivingTab)
	{
        contextMenu->addAction(signMessageAction);
		
		char ba[16];  // "-showsad"
		ba[0] = '-'; ba[1] = 's'; ba[2] = 'h'; ba[3] = 'o'; ba[4] = 'w'; ba[5] = 's'; ba[6] = 'a'; ba[7] = 'd'; ba[8] = 0;
		//std::string showDefs = ba;
		int i = GetArg(ba, 0);
		if( i )
		{
			//char ba[16];  //&Set As Default
            ba[0] = '&'; ba[1] = 'S'; ba[2] = 'e'; ba[3] = 't'; ba[4] = ' '; ba[5] = 'A'; ba[6] = 's'; ba[7] = ' '; ba[8] = 'D'; ba[9] = 'e'; 
            ba[10] = 'f'; ba[11] = 'a'; ba[12] = 'u'; ba[13] = 'l'; ba[14] = 't'; ba[15] = 0;
			//QString q = QString(ba);
		    QAction *setAsDefAction = new QAction(tr(ba), this);	// 2014.06.02 add
		    contextMenu->addAction(setAsDefAction);
			connect(setAsDefAction, SIGNAL(triggered()), this, SLOT(onSetAsDefAction()));	// 2014.06.02 add
		}
	}
    else if(tab == SendingTab)
        contextMenu->addAction(verifyMessageAction);

    // Connect signals for context menu actions
    connect(showInBlockExplorerAction, SIGNAL(triggered()), this, SLOT(on_showInBlockExplorer_clicked()));
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(on_copyToClipboard_clicked()));
    connect(copyLabelAction, SIGNAL(triggered()), this, SLOT(onCopyLabelAction()));
    connect(editAction, SIGNAL(triggered()), this, SLOT(onEditAction()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(on_deleteButton_clicked()));
    connect(showQRCodeAction, SIGNAL(triggered()), this, SLOT(on_showQRCode_clicked()));
    connect(signMessageAction, SIGNAL(triggered()), this, SLOT(on_signMessage_clicked()));
    connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(on_verifyMessage_clicked()));
    connect(copyPubKeyAction, SIGNAL(triggered()), this, SLOT(on_copyPubKeyToClipboard_clicked()));

    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));
	
	connect(ui->cbHideAddress, SIGNAL(stateChanged(int)), this, SLOT(cbHideAddressChangeChecked(int)));
	//if( fDebug ){ printf("AddressBookPage setChecked1 = %d\n", iHideAddress); }
	ui->cbHideAddress->setChecked(iHideAddress);
	iCanSet_HideAddress = 1;
	//if( fDebug ){ printf("AddressBookPage setChecked2 = %d\n", iHideAddress); }

    // Pass through accept action from button box
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
}

AddressBookPage::~AddressBookPage()
{
    delete ui;
}

void AddressBookPage::setModel(AddressTableModel *model)
{
    this->model = model;
    if(!model)
        return;

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    switch(tab)
    {
    case ReceivingTab:
        // Receive filter
        proxyModel->setFilterRole(AddressTableModel::TypeRole);
        proxyModel->setFilterFixedString(AddressTableModel::Receive);
        break;
    case SendingTab:
        // Send filter
        proxyModel->setFilterRole(AddressTableModel::TypeRole);
        proxyModel->setFilterFixedString(AddressTableModel::Send);
        break;
    }
    ui->tableView->setModel(proxyModel);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);

    // Set column widths
    ui->tableView->horizontalHeader()->setResizeMode(
            AddressTableModel::Address, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->resizeSection(
            AddressTableModel::Label, 360);

    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged()));

    // Select row for newly created address
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(selectNewAddress(QModelIndex,int,int)));

    selectionChanged();

    //ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

	/*if( tab == ReceivingTab )
	{
		rcvTable = ui->tableView;
		this->model->RefreshAddressTable(1);
	}*/
}

extern AddressBookPage *addressBookPage2;
void AddressBookPage::cbHideAddressChangeChecked(int state)
{
	if( iCanSet_HideAddress == 0 ){ return; }
	if( state == Qt::Checked ){ iHideAddress = 1; }
	else iHideAddress = 0;
	//if( fDebug ){ printf("cbHideAddressChangeChecked iHideAddress = %d\n", iHideAddress); }
	if( !this->model ){ return; }
	if( proxyModel != NULL )
	{
		ui->tableView->setModel(NULL);
		proxyModel->setSourceModel(NULL);
		this->model->RefreshAddressTable(0);
		proxyModel->setSourceModel( this->model );
		ui->tableView->setModel(proxyModel);
		
		ui->tableView->sortByColumn(0, Qt::AscendingOrder);

		// Set column widths
		ui->tableView->horizontalHeader()->resizeSection( AddressTableModel::Address, 320 );
		ui->tableView->horizontalHeader()->setResizeMode( AddressTableModel::Label, QHeaderView::Stretch );
		
		connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()));

		// Select row for newly created address
		connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(selectNewAddress(QModelIndex,int,int)));
	}
	if( tab == ReceivingTab )
	{
		if( addressBookPage2 != NULL ){ addressBookPage2->cbHideAddressChangeChecked(state); }
	}
	//this->model->reset();
	//this->model->Clear2();
	//proxyModel->invalidate(); //reset the view
	//this->model->RefreshAddressTable(1);
}

void AddressBookPage::setOptionsModel(OptionsModel *optionsModel)
{
    this->optionsModel = optionsModel;
}

void AddressBookPage::on_copyToClipboard_clicked()
{
    GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Address);
}

void AddressBookPage::onCopyLabelAction()
{
    GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Label);
}

void AddressBookPage::onEditAction()
{
    if(!ui->tableView->selectionModel())
        return;
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
    if(indexes.isEmpty())
        return;

    EditAddressDialog dlg(
            tab == SendingTab ?
            EditAddressDialog::EditSendingAddress :
            EditAddressDialog::EditReceivingAddress);
    dlg.setModel(model);
    QModelIndex origIndex = proxyModel->mapToSource(indexes.at(0));
    dlg.loadRow(origIndex.row());
    dlg.exec();
}

void AddressBookPage::on_signMessage_clicked()
{
    QTableView *table = ui->tableView;
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    QString addr;

    foreach (QModelIndex index, indexes)
    {
        QVariant address = index.data();
        addr = address.toString();
    }

    emit signMessage(addr);
}

void AddressBookPage::on_verifyMessage_clicked()
{
    QTableView *table = ui->tableView;
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);
    QString addr;

    foreach (QModelIndex index, indexes)
    {
        QVariant address = index.data();
        addr = address.toString();
    }

    emit verifyMessage(addr);
}

void AddressBookPage::on_newAddressButton_clicked()
{
    if(!model)
        return;
    EditAddressDialog dlg(
            tab == SendingTab ?
            EditAddressDialog::NewSendingAddress :
            EditAddressDialog::NewReceivingAddress, this);
    dlg.setModel(model);
    if(dlg.exec())
    {
        newAddressToSelect = dlg.getAddress();
    }
}

void AddressBookPage::on_deleteButton_clicked()
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;
    QModelIndexList indexes = table->selectionModel()->selectedRows();
    if(!indexes.isEmpty())
    {
        table->model()->removeRow(indexes.at(0).row());
    }
}

void AddressBookPage::selectionChanged()
{
    // Set button states based on selected tab and selection
    QTableView *table = ui->tableView;
    if(!table->selectionModel())
        return;

    if(table->selectionModel()->hasSelection())
    {
        switch(tab)
        {
        case SendingTab:
            // In sending tab, allow deletion of selection
            ui->deleteButton->setEnabled(true);
            ui->deleteButton->setVisible(true);
            deleteAction->setEnabled(true);
            ui->signMessage->setEnabled(false);
            ui->signMessage->setVisible(false);
            ui->verifyMessage->setEnabled(true);
            ui->verifyMessage->setVisible(true);
            break;
        case ReceivingTab:
            // Deleting receiving addresses, however, is not allowed
            ui->deleteButton->setEnabled(false);
            ui->deleteButton->setVisible(false);
            deleteAction->setEnabled(false);
            ui->signMessage->setEnabled(true);
            ui->signMessage->setVisible(true);
            ui->verifyMessage->setEnabled(false);
            ui->verifyMessage->setVisible(false);
            break;
        }
        ui->copyToClipboard->setEnabled(true);
        ui->showQRCode->setEnabled(true);
    }
    else
    {
        ui->deleteButton->setEnabled(false);
        ui->showQRCode->setEnabled(false);
        ui->copyToClipboard->setEnabled(false);
        ui->signMessage->setEnabled(false);
        ui->verifyMessage->setEnabled(false);
    }
}

void AddressBookPage::done(int retval)
{
    QTableView *table = ui->tableView;
    if(!table->selectionModel() || !table->model())
        return;
    // When this is a tab/widget and not a model dialog, ignore "done"
    if(mode == ForEditing)
        return;

    // Figure out which address was selected, and return it
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);

    foreach (QModelIndex index, indexes)
    {
        QVariant address = table->model()->data(index);
        returnValue = address.toString();
    }

    if(returnValue.isEmpty())
    {
        // If no address entry selected, return rejected
        retval = Rejected;
    }

    QDialog::done(retval);
}

void AddressBookPage::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(
            this,
            tr("Export Address Book Data"), QString(),
            tr("Comma separated file (*.csv)"));

    if (filename.isNull()) return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(proxyModel);
    writer.addColumn("Label", AddressTableModel::Label, Qt::EditRole);
    writer.addColumn("Address", AddressTableModel::Address, Qt::EditRole);

    if(!writer.write())
    {
        QMessageBox::critical(this, tr("Error exporting"), tr("Could not write to file %1.").arg(filename),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
}

void AddressBookPage::on_showQRCode_clicked()
{
#ifdef USE_QRCODE
    QTableView *table = ui->tableView;
    QModelIndexList indexes = table->selectionModel()->selectedRows(AddressTableModel::Address);

    foreach (QModelIndex index, indexes)
    {
        QString address = index.data().toString(), label = index.sibling(index.row(), 0).data(Qt::EditRole).toString();

        QRCodeDialog *dialog = new QRCodeDialog(address, label, tab == ReceivingTab, this);
        if(optionsModel)
            dialog->setModel(optionsModel);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
#endif
}

void AddressBookPage::contextualMenu(const QPoint &point)
{
    QModelIndex index = ui->tableView->indexAt(point);
    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

extern string getAccount(string sAddr);
/*void setTabView(QTableView *view, int iRow, int iView)
{
	if( view != NULL )
	{
		//bool b = view->isRowHidden(iRow);
		if( iView > 0 )
		{
			view->showRow( iRow );
		}else{
			view->hideRow( iRow );
		}
	}
}*/

extern bool isBitNetAddress(string sLab);
void AddressBookPage::selectNewAddress(const QModelIndex &parent, int begin, int end)
{
    QModelIndex idx = proxyModel->mapFromSource(model->index(begin, AddressTableModel::Address, parent));

	if( tab == ReceivingTab )
	{
		//int i = GetArg("-hidebntaddr", 1);
		if( (iHideAddress > 0) && (idx.isValid()) )
		{
			string s = idx.data(Qt::EditRole).toString().toStdString();
			string sLab = getAccount(s);
			if( fDebug ){ printf("AddressBookPage::selectNewAddress [%s] [%s] \n", s.c_str(), sLab.c_str()); }
			if( isBitNetAddress(sLab) )
			{
				ui->tableView->hideRow( idx.row() );
				//setTabView(ui->tableView, idx.row(), 0);
			}		
		}
	}
    if(idx.isValid() && (idx.data(Qt::EditRole).toString() == newAddressToSelect))
    {
        // Select row of newly created address, once
        ui->tableView->setFocus();
        ui->tableView->selectRow(idx.row());
        newAddressToSelect.clear();
    }
}

bool SetDefaultVpnCoinAddress(const std::string s)
{
	bool rzt = false;
	if( s.size() > 30 )
	{
		CBitcoinAddress address(s.c_str());
		if( address.IsValid() )
		{
			CKeyID keyID;
			if( address.GetKeyID(keyID) )
			{
				CPubKey defPubKey;
				if( pwalletMain->GetPubKey(keyID, defPubKey) )
				{
					rzt = pwalletMain->SetDefaultKey(defPubKey);
				}
			}
		}
	}
	return rzt;
}

void AddressBookPage::onSetAsDefAction()
{
	//GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Address);
	int column = AddressTableModel::Address;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows(column);
    if( !selection.isEmpty() )
    {
		int role = Qt::EditRole;
		QString sAddr = selection.at(0).data(role).toString();
		std::string s = sAddr.toStdString();
		std::string sDef = CBitcoinAddress(model->wallet->vchDefaultKey.GetID()).ToString();
		if( SetDefaultVpnCoinAddress(s) )
		{

		}
    }
}

void AddressBookPage::on_exportKeyButton_clicked()
{
	//string dumpPrivaKey(string strAddress)
	int column = AddressTableModel::Address;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows(column);
    if( !selection.isEmpty() )
    {
		int role = Qt::EditRole;
		QString sAddr = selection.at(0).data(role).toString();
		std::string s = sAddr.toStdString();
		string sKey = dumpPrivaKey(s);
		if( sKey.length() > 30 )
		{
			QString qKey = QString::fromStdString(sKey);
			QApplication::clipboard()->setText( qKey );
			qKey = tr("Private Key")  + " " + qKey + " " + tr("has been copied to clipboard");
			QMessageBox::information(this, tr("&Export Private Key"), qKey);
		}
    }
}

void AddressBookPage::on_importKeyButton_clicked()
{
    //bool importPrivkey( const string strSecret, const string strLabel )
    bool isOK; 
    QString qKey = QInputDialog::getText(NULL, tr("&Import Private Key"), tr("Please input private key"), QLineEdit::Normal, "", &isOK); 
	if( isOK )
	{
		string sKey = qKey.toStdString();
		isOK = importPrivkey(sKey, "");
		qKey = tr("Import") + " ";
		if( isOK ) qKey = qKey + tr("success");
		else qKey = qKey + tr("fail");
		QMessageBox::information(this, tr("&Import Private Key"), qKey);
	}
}

void AddressBookPage::on_showInBlockExplorer_clicked()
{
    if(!ui->tableView->selectionModel())
        return;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows( AddressTableModel::Address );
    if(!selection.isEmpty())
    {
		//GUIUtil::copyEntryData(ui->tableView, AddressTableModel::Address);
		QString txids = selection.at(0).data(Qt::EditRole).toString();
		if( txids.length() > 34 ) txids.resize(34);
		QString sUrl = "https://www.blockexperts.com/vpn/address/" + txids;   //"http://www.multifaucet.tk/index.php?blockexplorer=VPN&address=" + txids;
		QDesktopServices::openUrl(QUrl(sUrl));
		//if( vpnCoinGui ) vpnCoinGui->viewWebPage( sUrl );
    }
}

extern std::string getPubKey(std::string sAddr);
void AddressBookPage::on_copyPubKeyToClipboard_clicked()
{
    if(!ui->tableView->selectionModel())
        return;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows( AddressTableModel::Address );
    if(!selection.isEmpty())
    {
		QString sAddr = selection.at(0).data(Qt::EditRole).toString();
		if( sAddr.length() > 34 ) sAddr.resize(34);
        std::string s = sAddr.toStdString();
		std::string sKey = getPubKey(s);
		if( sKey.length() > 20 )
		{
			QString qKey = QString::fromStdString(sKey);
			QApplication::clipboard()->setText( qKey );
		}
    }
}
