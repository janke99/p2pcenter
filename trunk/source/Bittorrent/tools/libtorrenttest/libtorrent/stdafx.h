// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
// Windows 头文件:
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <iostream>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/peer_connection.hpp>

#include <windows.h>
#include <dde.h>
#include <shellapi.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <commdlg.h>
#include <strsafe.h>

#include "common.h"
// TODO: 在此处引用程序要求的附加头文件
