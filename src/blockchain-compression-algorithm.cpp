// Copyright (c) 2016 The Shenzhen BitChain Technology Company
// Copyright (c) 2016 Luckchain development team, Bit Lee

#include "wallet.h"
#include "walletdb.h"
#include "bitcoinrpc.h"
#include "init.h"
#include "base58.h"
#include "lz4/lz4.h"
#include "lzma/LzmaLib.h"
//#include "aes/Rijndael.h"
//#include "aes/aes.h"
//#include "bitchain.h"
#include <fstream>


#include <iostream>
#include <iterator>
#include <vector>

using namespace json_spirit;
using namespace std;
typedef char * PCHAR;

extern int dw_zip_block;
extern int dw_zip_limit_size;
unsigned int uint_256KB = 256 * 1024;

int StreamToBuffer(CDataStream &ds, string& sRzt, int iSaveBufSize)
{
	int bsz = ds.size();
	int iRsz = bsz;
	if( iSaveBufSize > 0 ){ iRsz = iRsz + 4; }
	sRzt.resize(iRsz);
	char* ppp = (char*)sRzt.c_str();
	if( iSaveBufSize > 0 ){ ppp = ppp + 4; }
	ds.read(ppp, bsz);
	if( iSaveBufSize > 0 ){ *(unsigned int *)(ppp - 4) = bsz; }
	return iRsz;
}

int CBlockToBuffer(CBlock *pb, string& sRzt)
{
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock << (*pb);
	/*int bsz = ssBlock.size();
	sRzt.resize(bsz);
	char* ppp = (char*)sRzt.c_str();
	ssBlock.read(ppp, bsz);*/
	int bsz = StreamToBuffer(ssBlock, sRzt, 0);
	return bsz;
}

int writeBufToFile(char* pBuf, int bufLen, string fName)
{
	int rzt = 0;
	std::ofstream oFs(fName.c_str(), std::ios::out | std::ofstream::binary);
	if( oFs.is_open() )
	{
		if( pBuf ) oFs.write(pBuf, bufLen);
		oFs.close(); 
		rzt++; 
	}
	return rzt;
}

int lz4_pack_buf(char* pBuf, int bufLen, string& sRzt)
{
	int worstCase = 0;
	int lenComp = 0;
    try{
		worstCase = LZ4_compressBound( bufLen );
		//std::vector<uint8_t> vchCompressed;   //vchCompressed.resize(worstCase);
		sRzt.resize(worstCase + 4);
		char* pp = (char *)sRzt.c_str();
		lenComp = LZ4_compress(pBuf, pp + 4, bufLen); 
		if( lenComp > 0 ){ *(unsigned int *)pp = bufLen;   lenComp = lenComp + 4; }
	}
    catch (std::exception &e) {
        printf("lz4_pack_buf err [%s]:: buf len %d, worstCase[%d], lenComp[%d] \n", e.what(), bufLen, worstCase, lenComp);
    }
	return lenComp;
}

int lz4_unpack_buf(const char* pZipBuf, unsigned int zipLen, string& sRzt)
{
	int rzt = 0;
	unsigned int realSz = *(unsigned int *)pZipBuf;
	if( fDebug )printf("lz4_unpack_buf:: zipLen [%d], realSz [%d],  \n", zipLen, realSz);
	sRzt.resize(realSz);
	char* pOutData = (char*)sRzt.c_str();
	
    // -- decompress
	rzt = LZ4_decompress_safe(pZipBuf + 4, pOutData, zipLen, realSz);
    if ( rzt != (int) realSz)
    {
            if( fDebug )printf("lz4_unpack_buf:: Could not decompress message data. [%d :: %d] \n", rzt, realSz);
            sRzt.resize(0);
    }
	return rzt;
}
int CBlockFromBuffer(CBlock* block, char* pBuf, int bufLen)
{
    //vector<char> v(bufLen);
	//memcpy((char*)&v[0], pBuf, bufLen);
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock.write(pBuf, bufLen);   int i = ssBlock.size();
	//ssBlock << v;   
	ssBlock >> (*block);
	return i;
}

int lz4_pack_block(CBlock* block, string& sRzt)
{
	int rzt = 0;
	string sbf;
	int bsz = CBlockToBuffer(block, sbf);
	if( bsz > 12 )
	{
		char* pBuf = (char*)sbf.c_str();
		rzt = lz4_pack_buf(pBuf, bsz, sRzt);
		//if( lzRzt > 0 ){ rzt = lzRzt; }  // + 4; }
	}
	sbf.resize(0);
	return rzt;
}

/* static void Uncompress1(
  std::vector<unsigned char> &outBuf,
  const std::vector<unsigned char> &inBuf)
{
  outBuf.resize(UNCOMPRESSED_SIZE);
  unsigned dstLen = outBuf.size();
  unsigned srcLen = inBuf.size() - LZMA_PROPS_SIZE;
  SRes res = LzmaUncompress(
    &outBuf[0], &dstLen,
    &inBuf[LZMA_PROPS_SIZE], &srcLen,
    &inBuf[0], LZMA_PROPS_SIZE);
  assert(res == SZ_OK);
  outBuf.resize(dstLen); // If uncompressed data can be smaller
}*/
int lzma_depack_buf(unsigned char* pLzmaBuf, int bufLen, string& sRzt)
{
	int rzt = 0;
	size_t dstLen = *(unsigned int *)pLzmaBuf;
    sRzt.resize(dstLen);
	unsigned char* pOutBuf = (unsigned char*)sRzt.c_str();
    size_t srcLen = bufLen - LZMA_PROPS_SIZE - 4;
	SRes res = LzmaUncompress(pOutBuf, &dstLen, &pLzmaBuf[LZMA_PROPS_SIZE + 4], &srcLen, &pLzmaBuf[4], LZMA_PROPS_SIZE);
	if( res == SZ_OK )//assert(res == SZ_OK);
	{
		//outBuf.resize(dstLen); // If uncompressed data can be smaller
		rzt = dstLen;
	}else sRzt.resize(0);
	//if( fDebug ) printf("lzma_depack_buf:: res [%d], dstLen[%d],  rzt = [%d]\n", res, dstLen, rzt);
	return rzt;
}

int lzma_pack_buf(unsigned char* pBuf, int bufLen, string& sRzt, int iLevel, unsigned int iDictSize)  // (1 << 17) = 131072 = 128K
{
	int res = 0;
	int rzt = 0;
		size_t propsSize = LZMA_PROPS_SIZE,    szSrcLen = bufLen;
		size_t destLen = bufLen + (bufLen / 3) + 128;
    try{
		sRzt.resize(propsSize + destLen + 4);
		unsigned char* pOutBuf = (unsigned char*)sRzt.c_str();

		res = LzmaCompress(&pOutBuf[LZMA_PROPS_SIZE + 4], &destLen, pBuf, szSrcLen, &pOutBuf[4], &propsSize,
                                      iLevel, iDictSize, -1, -1, -1, -1, -1);  // 1 << 14 = 16K, 1 << 16 = 64K
  
		//assert(propsSize == LZMA_PROPS_SIZE);
		//assert(res == SZ_OK);
		if( (res == SZ_OK) && (propsSize == LZMA_PROPS_SIZE) )  
		{
			//outBuf.resize(propsSize + destLen);
			*(unsigned int *)pOutBuf = bufLen; 
			rzt = propsSize + destLen + 4;
		}else sRzt.resize(0);
	
	}
    catch (std::exception &e) {
        printf("lzma_pack_buf err [%s]:: buf len %d, rzt[%d] \n", e.what(), bufLen, rzt);
    }
	//if( fDebug ) printf("lzma_pack_buf:: res [%d], propsSize[%d], destLen[%d],  rzt = [%d]\n", res, propsSize, destLen, rzt);
	return rzt;
}

int lzma_pack_block(CBlock* block, string& sRzt, int iLevel, unsigned int iDictSize)  // (1 << 17) = 131072 = 128K
{
	int rzt = 0;
	string sbf;
	int bsz = CBlockToBuffer(block, sbf);
	if( bsz > 12 )
	{
		unsigned char* pBuf = (unsigned char*)sbf.c_str();
		rzt = lzma_pack_buf(pBuf, bsz, sRzt, iLevel, iDictSize);
		//if( lzRzt > 0 ){ rzt = lzRzt; }  // + 4; }
	}
	sbf.resize(0);
	return rzt;
}

void lzma_Block_to_file(CBlock *block, string sFile)
{
	string lzBuf;
	int64_t i61 = GetTimeMillis();
	int lzRzt = lzma_pack_block(block, lzBuf, 9, uint_256KB);
	int64_t i62 = GetTimeMillis();
	printf("lzma_Block_to_file:: lzma_pack_block used time (%I64d ~ %I64d) = [%I64d] \n", i61, i62, i62 - i61);
	if( lzRzt > 0 )
	{
		string sss = sFile + ".lzma";
		char* pBuf = (char*)lzBuf.c_str();
		writeBufToFile(pBuf, lzRzt, sss);
		
		string sDe;
		int lzRzt = lzma_depack_buf((unsigned char*)pBuf, lzRzt, sDe);
		if( lzRzt > 0 )
		{
			sss = sFile + ".unlzma";
			pBuf = (char*)sDe.c_str();
			writeBufToFile(pBuf, lzRzt, sss);
		}
	}
}

int SaveBlockToFile(CBlock* block, int iHi, int iTeam)
{
	int rzt = 0;
	string sFile, sFBack;
	int64_t i6Start = GetTimeMillis();
	//if( iTeam == 0 ){ sFile = strprintf("%s%d.block", s_BlockChain_Dir.c_str(), iHi); }
    if( iTeam == 0 ){ sFile = strprintf("%s\\block\\%d", s_BlockChain_Dir.c_str(), iHi); }
	else{
		int i = 0, ib = 0;
		if( iHi > 0 ){ i = (iHi-1) / iTeam; }   //int i = iHi / iTeam;  // 1 / 1000
		if( i > 0 ){ ib = i * iTeam; ib++; }
		//sFile = strprintf("%s%d-%d", s_BlockChain_Dir.c_str(), (i * iTeam), (i* iTeam + iTeam - 1));
		sFile = strprintf("%s\\block\\%d-%d", s_BlockChain_Dir.c_str(), ib, (i* iTeam + iTeam));
		boost::filesystem::create_directory(sFile);
		sFile = strprintf("%s\\%d", sFile.c_str(), iHi); 
	}
	sFBack = sFile.c_str();
	FILE* file = fopen(sFile.c_str(), "wb");
	if (!file){ return rzt; }
	CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
	if( !fileout ){ return rzt; }

    uint32_t lenMsg = fileout.GetSerializeSize(*block);  //message.size(); 
	
	lzma_Block_to_file(block, sFile);

	int64_t i62 = GetTimeMillis();
	printf("SaveBlockToFile:: lzma save block (%I64d ~ %I64d) = [%I64d] \n", i6Start, i62, (i62 - i6Start));

	string sss = sFile + ".blk";
	string sbf;
	/*CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock << (*block);
	int bsz = ssBlock.size();
	sbf.resize(bsz);
	char* ppp = (char*)sbf.c_str();
	ssBlock.read(ppp, bsz);*/
	int bsz = CBlockToBuffer(block, sbf);
	
	i62 = GetTimeMillis();
	printf("SaveBlockToFile:: CBlockToBuffer (%I64d ~ %I64d) = [%I64d] \n", i6Start, i62, (i62 - i6Start));

	char* pBuf = (char*)sbf.c_str();
	//writeBufToFile(pBuf, sbf.length(), sss);
	sss = sFile;
	string lzBuf;

	int64_t i61 = GetTimeMillis();
	int lzRzt = lz4_pack_buf(pBuf, bsz, lzBuf);
	i62 = GetTimeMillis();
	printf("SaveBlockToFile:: lz4_pack_buf used time (%I64d ~ %I64d) = [%I64d], i6Start = [%I64d] \n", i61, i62, i62 - i61, i6Start);

	if( lzRzt > 0 )
	{
		sss = sss + ".lz4";   pBuf = (char*)lzBuf.c_str();
		writeBufToFile(pBuf, lzRzt, sss);

		i62 = GetTimeMillis();
		printf("SaveBlockToFile:: save lz4 buf (%I64d ~ %I64d) = [%I64d] \n", i6Start, i62, (i62 - i6Start));

		string sUnpak;
		lzRzt = lz4_unpack_buf( pBuf, lzRzt - 4, sUnpak);
		
		i62 = GetTimeMillis();
		printf("SaveBlockToFile::lz4_unpack_buf (%I64d ~ %I64d) = [%I64d] \n", i6Start, i62, (i62 - i6Start));

		if( lzRzt > 0 )
		{
			sss = sss + ".unlz4";   pBuf = (char*)sUnpak.c_str();
			writeBufToFile(pBuf, lzRzt, sss);
			
			i62 = GetTimeMillis();
			printf("SaveBlockToFile::save lz4_unpack_buf (%I64d ~ %I64d) = [%I64d] \n", i6Start, i62, (i62 - i6Start));

			sUnpak.resize(0);
		}
	}
	sbf.resize(0);   lzBuf.resize(0);

    // Write index header
    //unsigned int nSize = fileout.GetSerializeSize(*block);
    //fileout << FLATDATA(pchMessageStart) << nSize;
		
    fileout << *block;
    fflush(fileout);
    FileCommit(fileout);
	fileout.fclose();
	rzt++;
	
    std::string lzFile = sFile + ".lz";
	int worstCase = 0;
	int lenComp = 0;
    try{
	worstCase = LZ4_compressBound( lenMsg );
    //std::vector<uint8_t> vchCompressed;   //vchCompressed.resize(worstCase);
	string rzs;   rzs.resize(worstCase);
	printf("SaveBlockToFile:: 1 block len %d, worstCase[%d], lenComp[%d] \n", lenMsg, worstCase, lenComp);
	
	ifstream is;
	is.open( sFBack.c_str(), ios::binary);
	//if( !is.is_open() ){ return rzt; }
	if( is.is_open() )
	{
		//printf("ReadBlockIndexFromFile open ok [%s] \n", block.ToString().c_str());
		//OutputDebugStringA("Open ok");
		//string s;
		//s.resize(iMaxSz);
		char* pchMsgTmp = new char[lenMsg];
		//unsigned int lenData = 
		is.read(&pchMsgTmp[0], lenMsg);  //is.read((char*)s.c_str(), iMaxSz);
		unsigned int lenData = is.gcount();
		is.close();

	//int LZ4_compress(const char* source, char* dest, int inputSize) { return LZ4_compress_default(source, dest, inputSize, LZ4_compressBound(inputSize)); }
    //lenComp = LZ4_compress((char*)block, (char *)rzs.c_str(), lenMsg);   //lenComp = LZ4_compress((char*)block, (char*)&vchCompressed[0], lenMsg); 
	lenComp = LZ4_compress(&pchMsgTmp[0], (char *)rzs.c_str(), lenMsg);   //lenComp = LZ4_compress((char*)block, (char*)&vchCompressed[0], lenMsg); 
	//lenComp = LZ4_compress_default((const char*)block, (char *)rzs.c_str(), lenMsg, worstCase); 
	delete[] pchMsgTmp;
	printf("SaveBlockToFile:: 2 block len %d, worstCase[%d], lenComp[%d] \n", lenMsg, worstCase, lenComp);
	if( lenComp > 0 )
	{
	char* pMsgData = (char *)rzs.c_str();  //uint8_t* pMsgData = &vchCompressed[0];
	//ofstream oFs;
    //oFs.open(lzFile.c_str(), ios::binary);
	//if( !is.is_open() ){ return rzt; }
	std::ofstream oFs(lzFile.c_str(), std::ios::out | std::ofstream::binary);
	if( oFs.is_open() )
	{
		if( pMsgData ) oFs.write((const char*)pMsgData, lenComp);  //oFs << vchCompressed;
		//std::copy(vchCompressed.begin(), vchCompressed.end(), std::ostreambuf_iterator<char>(oFs));  // test ok
		oFs.close(); 
		//rzt++; 
	}
	rzs.resize(0);
	}
	//os.write( (const char*)&vchCompressed[0], sizeof(vchCompressed) );
    //std::ofstream FILE(lzFile, std::ios::out | std::ofstream::binary);
    //std::copy(vchCompressed.begin(), vchCompressed.end(), std::ostreambuf_iterator<char>(FILE));
    }
	}
    catch (std::exception &e) {
        printf("SaveBlockToFile err [%s]:: block len %d, worstCase[%d], lenComp[%d] \n", e.what(), lenMsg, worstCase, lenComp);
    }

	/*ofstream myfile; 
	myfile.open (sFile.c_str(), std::ofstream::binary);
	if( myfile.is_open() )
	{
		myfile.write((const char*)&block, sizeof(CBlock));
		myfile.close(); 
		rzt++; 
	}*/
	return rzt;
}

Value saveblocktofile(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1)
	{
		throw runtime_error(
            "saveblocktofile <height> <option> >\n");
	}
	string rzt = "0";
	int iHi = params[0].get_int();
	int iType  = 0;
	if( params.size() > 1 ){ iType = params[1].get_int(); }
	if( iHi >= 0 )
	{
		CBlockIndex* pblockindex = FindBlockByHeight(iHi);
		if( pblockindex )
		{
			if( iType == 0 )
			{
				CBlock block;
				block.ReadFromDisk(pblockindex);
				block.print();
				SaveBlockToFile(&block, iHi, 0);   rzt = "1";
			}/*else if( iType == 1 ){
				SaveBlockIndexToFile(pblockindex, iHi);   rzt = "1";
			}
			else if( iType == 2 ){
				CDiskBlockIndex cIdx(pblockindex);
				SaveCBlockIndexToFile(cIdx, iHi);   rzt = "1";
			}*/
		}
	}
	return rzt;
}

int bitnet_pack_block(CBlock* block, string& sRzt)
{
	if( dw_zip_block == 1 )  return lzma_pack_block(block, sRzt, 9, uint_256KB);
	else if( dw_zip_block == 2 ) return lz4_pack_block(block, sRzt);
}

bool readZipBlockFromDisk(const CBlockIndex* pindex, std::string& sRzt)
{
    bool fReadTransactions = true;   sRzt = "";
    if( !pindex ){ return false; }
	unsigned int fPos = 0;
	//if( dw_zip_block > 0 ){ nBlockPos = 0; }

    // Open history file to read
    CAutoFile filein = CAutoFile(OpenBlockFile(pindex->nFile, fPos, "rb"), SER_DISK, CLIENT_VERSION);
    if( !filein ){ return false; }  //error("CBlock::ReadFromDisk() : OpenBlockFile failed");
    if( !fReadTransactions ){ filein.nType |= SER_BLOCKHEADERONLY; }

	int ips = pindex->nBlockPos - 4;  // get ziped block size;
	if (fseek(filein, ips, SEEK_SET) != 0)
		return false;   //error("getCBlockByFilePos:: fseek failed");
	filein >> ips;	// get ziped block size;
	sRzt.resize(ips);   char* pZipBuf = (char *)sRzt.c_str();
	filein.read(pZipBuf, ips);

    //if (!ReadFromDisk(pindex->nFile, pindex->nBlockPos, fReadTransactions)){ return false; }
    //if (GetHash() != pindex->GetBlockHash())
    //    return error("CBlock::ReadFromDisk() : GetHash() doesn't match index");
    return true;
}

bool getCBlockByFilePos(const CAutoFile& filein, unsigned int nBlockPos, CBlock* block)
{
	bool rzt = false;
//printf("getCBlockByFilePos:: ---> \n");
	int ips = nBlockPos - 4;  // get ziped block size;
	if (fseek((CAutoFile&)filein, ips, SEEK_SET) != 0)
		return error("getCBlockByFilePos:: fseek failed");
	(CAutoFile&)filein >> ips;	// get ziped block size;
	//if( fDebug )printf("getCBlockByFilePos:: ziped block size [%d] \n", ips);
	string s;   s.resize(ips);   char* pZipBuf = (char *)s.c_str();
	((CAutoFile&)filein).read(pZipBuf, ips);
	string sUnpak = "";
	int iRealSz;
	if( dw_zip_block == 1 ) iRealSz = lzma_depack_buf((unsigned char*)pZipBuf, ips, sUnpak);
	else if( dw_zip_block == 2 ) iRealSz = lz4_unpack_buf(pZipBuf, ips - 4, sUnpak);
	//if( fDebug )printf("getCBlockByFilePos:: ziped block size [%d], iRealSz [%d] \n", ips, iRealSz);
	if( iRealSz > 0 )
	{
		pZipBuf = (char *)sUnpak.c_str();
		rzt = CBlockFromBuffer(block, pZipBuf, iRealSz) > 12;
		/*if( fDebug ){
			if( block->vtx.size() < 10 )
			{
				printf("\n\n getCBlockByFilePos:: block info (%d): \n", rzt);
				block->print(); 
			}else printf("\n\n getCBlockByFilePos:: block vtx count (%d) is too large \n", block->vtx.size());
		}*/
	}
	s.resize(0);   sUnpak.resize(0);
//printf("getCBlockByFilePos:: [%d] <--- \n", rzt);
	return rzt;
}

bool getCBlocksTxByFilePos(const CAutoFile& filein, unsigned int nBlockPos, unsigned int txId, CTransaction& tx)
{
	bool rzt = false;
//printf("getCBlocksTxByFilePos:: ---> [%X] \n", filein);
	CBlock block;
	rzt = getCBlockByFilePos(filein, nBlockPos, &block);
	if( rzt )
	{
		if( block.vtx.size() > txId )
		{
			tx = block.vtx[txId];
			//if( fDebug ){	printf("\n\n getCBlocksTxByFilePos:: tx info: \n");   tx.print(); }
		}else rzt = false;
	}
//printf("getCBlocksTxByFilePos:: [%d] [%X]<--- \n", rzt, filein);
	return rzt;
}

int bitnet_pack_buf(char* pBuf, int bufLen, string& sRzt)
{
    if( dw_zip_block == 1 ) return lzma_pack_buf((unsigned char *)pBuf, bufLen, sRzt, 9, uint_256KB);
	else if( dw_zip_block == 2 ) return lz4_pack_buf((char *)pBuf, bufLen, sRzt);
}
int bitnet_depack_buf(char* pLzBuf, int lzBufLen, string& sRzt)
{
    if( dw_zip_block == 1 ) return lzma_depack_buf((unsigned char *)pLzBuf, lzBufLen, sRzt);
	else if( dw_zip_block == 2 ) return lz4_unpack_buf(pLzBuf, lzBufLen, sRzt);
}
void bitnet_depack_Db_DataStream(CDataStream& ssValue)
{
			if( dw_zip_block > 0 )
			{
				//char* ps = (char *)&ssValue[0];  // ssValue.size()
				int isz = ssValue.size();
				if( isz < 12 ){ return; }

				string sV, sRzt;
				int bufLen = StreamToBuffer(ssValue, sV, 0);
				char* pLzBuf = (char*)sV.c_str();
				ssValue.clear(); 

				unsigned int iSv = *(unsigned int *)pLzBuf;  // Orig buf size
				unsigned int iDictSz = *(unsigned int *)(pLzBuf + 5);
				printf("bitnet_depack_Db_DataStream:: iSv [%u :: %u], iDictSz [%u], pLzBuf[3] = (%u) \n", iSv, isz, iDictSz, pLzBuf[3]);
				//if( (ps[3] == 0) && (ps[4] == 0x5D) && (ps[5] == 0) && (iSv >= ssValue.size()) )  // by lzma compress flag
				if( (pLzBuf[3] == 0) && (iDictSz == uint_256KB) && (iSv >= isz) )  // by lzma compress flag
				{
					//iSv = StreamToBuffer(ssValue, sV, 0);
					
					unsigned int iSz = *(unsigned int *)pLzBuf;
					//ssValue.clear(); 
					/*if( iSz < dw_zip_limit_size )  // dw_zip_limit_size default = 64
					{
						pLzBuf = pLzBuf + 4;
						ssValue.write(pLzBuf, iSz);   //ssValue >> value;
					}else*/
					{
						if( dw_zip_block == 1 )
						{
							int iLz = bitnet_depack_buf(pLzBuf, iSv, sRzt);
							if( iLz > 12 )
							{
								//CDataStream ss2(SER_DISK, CLIENT_VERSION);
								//ss2.write((char *)sRzt.c_str(), iLz);   //ss2 >> value;
								ssValue.write((char *)sRzt.c_str(), iLz);   //ssValue >> value;
							}
						}
						else if( dw_zip_block == 2 ){
					
						}
					}
					sRzt.resize(0);
				}else ssValue.write(pLzBuf, bufLen);
				sV.resize(0);
			}
}

void bitnet_pack_Db_DataStream(CDataStream& ssValue)
{
		if( dw_zip_block > 0 )
		{
			int iSaveBufSize = 0;
			int iSz = ssValue.size();
			if( iSz < dw_zip_limit_size ){  // dw_zip_limit_size default = 168
				return;  //iSaveBufSize++;
			}

			string sV, sRzt;
			int bufLen = StreamToBuffer(ssValue, sV, iSaveBufSize);
			char* pLzBuf = (char*)sV.c_str();
			ssValue.clear();
			if( iSaveBufSize == 0 )
			{
				if( dw_zip_block == 1 )
				{
					int iLz = bitnet_pack_buf(pLzBuf, bufLen, sRzt);
					if( (iLz > 12) && (iLz < iSz) )
					{
						ssValue.write((char *)sRzt.c_str(), iLz);
					}else ssValue.write(pLzBuf, bufLen);
				}
				else if( dw_zip_block == 2 ){
			
				}
			}else{  // less than dw_zip_limit_size(64) byte, did not need zip it
				ssValue.write(pLzBuf, bufLen);
			}
			sV.resize(0);   sRzt.resize(0);
		}
}

Value lzmablock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 5)
	{
		throw runtime_error(
            "lzmablock <height> <lzma level> <lzma dict size><class type (CBlock = 1,  CDiskBlockIndex = 2)><algorithm(Lzma = 1, Lz4 = 2)>\n");
	}
	//string rzt = "0";
	int iHi = params[0].get_int();
	int iLevel = params[1].get_int();
	unsigned int iDictSize = 131072;  // 128K
	if( params.size() > 2 ){ iDictSize = params[2].get_int() * 1024; }
	int iCtype = params[3].get_int();
	int iAlg = params[4].get_int();
	//int iType  = 0;
	//if( params.size() > 1 ){ iType = params[1].get_int(); }
    Object ret;
	ret.push_back( Pair("Height", iHi) );
	ret.push_back( Pair("Level", iLevel) );
	ret.push_back( Pair("Dict Size", (int)iDictSize) );
	if( iCtype < 1 ){ iCtype = 1; }
	if( iCtype == 1 ){ ret.push_back( Pair("Class Type", "CBlock") ); }
	else if( iCtype == 1 ){ ret.push_back( Pair("Class Type", "CDiskBlockIndex") ); }
	if( iAlg == 1 ){ ret.push_back( Pair("Algorithm", "Lzma") ); }
	else if( iAlg == 2 ){ ret.push_back( Pair("Algorithm", "Lz4") ); }
	if( iHi >= 0 )
	{
		CBlockIndex* pblockindex = FindBlockByHeight(iHi);
		if( pblockindex )
		{
			string lzBuf;
			int64_t i61 = GetTimeMillis();
			ret.push_back( Pair("Start", i61) );

			if(iCtype == 2 )  //CDiskBlockIndex
			{
				CDiskBlockIndex dbi(pblockindex);
				CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
				ssBlock << dbi;
				int bsz = StreamToBuffer(ssBlock, lzBuf, 0);
				ret.push_back( Pair("Base size", bsz) );
				unsigned char* pBuf = (unsigned char*)lzBuf.c_str();
				string sRzt;
				if( iAlg == 1 ) // Lzma
				{
					bsz = lzma_pack_buf(pBuf, bsz, sRzt, iLevel, iDictSize);
				}else{  // Lz4
					bsz = lz4_pack_buf((char*)pBuf, bsz, sRzt);
				}
				ret.push_back( Pair("Compress size", bsz) );
				sRzt.resize(0);
			}else{  // CBlock
			
			CBlock block;
			block.ReadFromDisk(pblockindex);
			//block.print();
//int lzma_pack_block(CBlock* block, string& sRzt, int iLevel, unsigned iDictSize)  // (1 << 17) = 131072 = 128K
			
			int lzRzt = lzma_pack_block(&block, lzBuf, iLevel, iDictSize);
			ret.push_back( Pair("Compress size", lzRzt) );
			if( lzRzt > 0 )
			{
				int bSz = *(int *)lzBuf.c_str();
				ret.push_back( Pair("Base size", bSz) );
			}}
			int64_t i62 = GetTimeMillis();
			ret.push_back( Pair("End", i62) );
			lzBuf.resize(0);
		}
	}
	return ret;
}

Value decodelzfile(const Array& params, bool fHelp)   //decodelzfile c:/coin/BitNet/lz4/0.lz
{
    if (fHelp || params.size() < 1)
	{
		throw runtime_error(
            "decodelzfile <filename> <max size>\n");
	}
	unsigned int iMaxSz = 128 * 1024;
	if( params.size() > 1 ) iMaxSz = params[1].get_int();
	
	string sFile = params[0].get_str();
	//string sFile = strprintf("%s%d.idx", s_BlockChain_Dir.c_str(), iHi);

	ifstream is;
	is.open( sFile.c_str(), ios::binary);
	//if( !is.is_open() ){ return rzt; }
	if( is.is_open() )
	{
		//printf("ReadBlockIndexFromFile open ok [%s] \n", block.ToString().c_str());
		//OutputDebugStringA("Open ok");
		//string s;
		//s.resize(iMaxSz);
		char* pchMsgTmp = new char[iMaxSz];
		//unsigned int lenData = 
		is.read(&pchMsgTmp[0], iMaxSz);  //is.read((char*)s.c_str(), iMaxSz);
		unsigned int lenData = is.gcount();
		is.close();

		char* p = (char *)&pchMsgTmp[0];  //s.c_str();
		char* pLz4Data = p + 4;  //((char *)block) + 4;
		string sBuf;
		unsigned int lenPlain = *(unsigned int *)p;
		sBuf.resize(lenPlain);
		char* pOutData = (char *)sBuf.c_str();
		// -- decompress
		int iLzRzt = LZ4_decompress_safe((const char*) pLz4Data, pOutData, lenData, lenPlain);
		delete[] pchMsgTmp;  //s.resize(0);
		if( iLzRzt != (int) lenPlain )
		{
            sBuf.resize(0);
			string st = strprintf("decodelzfile:: Could not decompress message data. (%d :: %d :: %d)", lenData, iLzRzt, lenPlain);
            return st;
		}else{

			sFile = sFile + ".unlz4";
			std::ofstream myfile(sFile.c_str(), std::ios::out | std::ofstream::binary);
			if( myfile.is_open() )
			{
				myfile.write(pOutData, iLzRzt);
				myfile.close();  sBuf.resize(0);
				return sFile;
			}else return "Can't write " + sFile;
		}

	}else return "Can't open " + sFile;
}
