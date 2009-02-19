// libtorrenttest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../libtorrent/IBittorrent.h"
#include <conio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc < 2)
	{
		printf("par error.\nright: libtorrenttest.exe <.torrent file filepath>\n");
		return -1;
	}

	IBittorrent* bittorrent =CreateIBittorrent();
	if ( bittorrent->InitModule( 0))
	{
		printf("Init succeed.\n");
		DWORD torrent =bittorrent->OpenSource( argv[1], false);
		//DWORD torrent =bittorrent->OpenSource( "C:\\bbs[1].wofei.net@穿越大吉岭.torrent", false);
		printf("open torrent: %d\n", torrent);

		while ( 1 )
		{
			if ( kbhit() )
			{
				char key = getch();

				if ( key == 27 )
				{
					break;
				}
			}
			bittorrent->tick_it();
			Sleep(50);
		}

		bittorrent->ReleaseModule();
	}
	return 0;
}

