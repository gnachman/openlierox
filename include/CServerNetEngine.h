/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


#ifndef __CSERVER_NET_ENGINE_H__
#define __CSERVER_NET_ENGINE_H__

#include "CWorm.h"
#include "CVec.h"

class GameServer;
class CServerConnection;

class CServerNetEngine
{
public:
	// Constructor
	CServerNetEngine( GameServer * _server, CServerConnection * _client ):
		server( _server ), cl( _client )
		{ }

	virtual ~CServerNetEngine() { }
	
	// TODO: move here all net code from CServer
	// Do not move here ParseConnectionlessPacket(), or make it static, 'cause client is not available for connectionless packet
	
	// Parsing

	void		ParsePacket(CBytestream *bs);

	virtual void ParseChatCommandCompletionRequest(CBytestream *bs) { return; };
	virtual void ParseAFK(CBytestream *bs) { return; };

	void		ParseImReady(CBytestream *bs);
	void		ParseUpdate(CBytestream *bs);
	void		ParseDeathPacket(CBytestream *bs);
	void		ParseChatText(CBytestream *bs);
	void		ParseUpdateLobby(CBytestream *bs);
	void		ParseDisconnect();
	void		ParseGrabBonus(CBytestream *bs);
	void		ParseSendFile(CBytestream *bs);

	bool		ParseChatCommand(const std::string& message);

	// Sending
	
	void		SendPacket(CBytestream *bs);

	void SendPrepareGame();
	virtual void SendText(const std::string& text, int type);
	virtual void SendChatCommandCompletionSolution(const std::string& startStr, const std::string& solution) { return; };
	virtual void SendChatCommandCompletionList(const std::string& startStr, const std::list<std::string>& solutions) { return; };
	virtual int SendFiles() { return 0; }; // Returns client ping, or 0 if no packet was sent

	void SendClientReady(CServerConnection* receiver); // If Receiver != NULL we're sending to worm connected during game
	void SendWormsOut(const std::list<byte>& ids);
	void SendWormDied(CWorm *Worm);
	void SendWormScore(CWorm *Worm);
	void SendWeapons();
	void SendSpawnWorm(CWorm *Worm, CVec pos);

protected:
	// Attributes
	GameServer 	*server;
	CServerConnection *cl;
	
	virtual void WritePrepareGame(CBytestream *bs);
};

class CServerNetEngineBeta3: public CServerNetEngine 
{

public:
	CServerNetEngineBeta3( GameServer * _server, CServerConnection * _client ):
		CServerNetEngine( _server, _client )
		{ }

	void SendText(const std::string& text, int type);
};

class CServerNetEngineBeta5: public CServerNetEngineBeta3
{

public:
	CServerNetEngineBeta5( GameServer * _server, CServerConnection * _client ):
		CServerNetEngineBeta3( _server, _client )
		{ }
		
	int SendFiles();
};

class CServerNetEngineBeta7: public CServerNetEngineBeta5
{

public:
	CServerNetEngineBeta7( GameServer * _server, CServerConnection * _client ):
		CServerNetEngineBeta5( _server, _client )
		{ }

	void ParseChatCommandCompletionRequest(CBytestream *bs);
	void ParseAFK(CBytestream *bs);

	void SendChatCommandCompletionSolution(const std::string& startStr, const std::string& solution);
	void SendChatCommandCompletionList(const std::string& startStr, const std::list<std::string>& solutions);

protected:
	void WritePrepareGame(CBytestream *bs);
};

class CServerNetEngineBeta8: public CServerNetEngineBeta7
{

public:
	CServerNetEngineBeta8( GameServer * _server, CServerConnection * _client ):
		CServerNetEngineBeta7( _server, _client )
		{ }

	void SendText(const std::string& text, int type);
};

class CServerNetEngineBeta9: public CServerNetEngineBeta8
{

public:
	CServerNetEngineBeta9( GameServer * _server, CServerConnection * _client ):
		CServerNetEngineBeta8( _server, _client )
		{ }

protected:
	void WritePrepareGame(CBytestream *bs);
};

#endif  //  __CSERVER_NET_ENGINE_H__
