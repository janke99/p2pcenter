#include "stdafx.h"
#include ".\bittorrent.h"
#include "resource.h"
#include <objbase.h>
using std::pair;
using std::sort;
using std::vector;
using std::string;
using std::string;
using std::auto_ptr;
using std::exception;
using boost::filesystem::path;
using namespace boost::spirit;
using libtorrent::entry;

static const UINT columns[]={
	IDS_NAME, IDS_SIZE, IDS_DOWNLOADED, IDS_UPLOADED,
		IDS_STATUS, IDS_PROGRESS, IDS_DOWNSPEED, IDS_UPSPEED,
		IDS_HEALTH, IDS_SEEDS, IDS_PEERS
};
static const size_t columncount=sizeof(columns)/sizeof(UINT);

struct Torrent {
	std::string file;
	libtorrent::torrent_handle handle;

	string cols[columncount];

	bool operator<(const Torrent &t) const {
		return stricmp(cols[0].c_str(), t.cols[0].c_str())<0;
	}
};

class Configuration {
public:
	int uplimit, downlimit;
	int firstport, lastport;
	int maxcon, torrentmaxcon, maxup;
	boost::filesystem::path savepath;

	Configuration::Configuration() :
	uplimit(-1),downlimit(-1),firstport(6881),lastport(6889),maxup(13),
		maxcon(200),torrentmaxcon(140) {
			char folder[MAX_PATH];
			SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, folder);

			boost::filesystem::path::default_name_check(boost::filesystem::native);

			this->savepath=folder;
			this->savepath/="btdowns";
		}

		bool Configuration::Load() {
			try {
				entry config=bdecode(getmodulepath()/"libtorrent.conf");
				entry::dictionary_type &config_d=config.dict();

				for(entry::dictionary_type::const_iterator iter=config_d.begin(); iter!=config_d.end(); iter++) {
					if(iter->first=="network") {
						const entry::dictionary_type &network=iter->second.dict();
						for(entry::dictionary_type::const_iterator niter=network.begin(); niter!=network.end(); niter++) {
							if(niter->first=="uplimit") this->uplimit=(int)niter->second.integer();
							else if(niter->first=="downlimit") this->downlimit=(int)niter->second.integer();
							else if(niter->first=="firstport") this->firstport=(int)niter->second.integer();
							else if(niter->first=="lastport") this->lastport=(int)niter->second.integer();
						}
					}
					else if(iter->first=="filesystem") {
						const entry::dictionary_type &filesystem=iter->second.dict();
						for(entry::dictionary_type::const_iterator fiter=filesystem.begin(); fiter!=filesystem.end(); fiter++) {
							if(fiter->first=="savepath") this->savepath=fiter->second.string();
						}
					}
				}

				if(this->uplimit>-1 && this->uplimit<12) this->downlimit=this->uplimit*5;
			}
			catch(...) {
				return false;
			}

			return true;
		}

		bool Configuration::Save() {
			entry::dictionary_type config;

			{
				entry::dictionary_type network;
				network.push_back(pair<string,entry>("uplimit", this->uplimit));
				network.push_back(pair<string,entry>("downlimit", this->downlimit));
				network.push_back(pair<string,entry>("firstport", this->firstport));
				network.push_back(pair<string,entry>("lastport", this->lastport));

				config.push_back(pair<string,entry>("network", network));
			}

			{
				entry::dictionary_type filesystem;
				filesystem.push_back(pair<string,entry>("savepath", this->savepath.native_directory_string()));

				config.push_back(pair<string,entry>("filesystem", filesystem));
			}

			return bencode(getmodulepath()/"libtorrent.conf", config);
		}
};

static libtorrent::session *session=NULL;
static vector<Torrent> torrents;
static bool allpaused=false;
Configuration conf;

static string paused;
static string queued;
static string checking;
static string connecting;
static string downloading;
static string seeding;
static string unknown;
static string bytes;
static string kibibytes;
static string mebibytes;
static string gibibytes;

static char *strsize(double s) {
	double downloaded;
	const char *units;
	if(s>=1073741824) {
		downloaded=s/1073741824.0;
		units=gibibytes.c_str();
	}
	else if(s>=1048576) {
		downloaded=s/1048576.0;
		units=mebibytes.c_str();
	}
	else if(s>=1024) {
		downloaded=s/1024.0;
		units=kibibytes.c_str();
	}
	else {
		downloaded=s;
		units=bytes.c_str();
	}

	static char buf[64];
	StringCchPrintf(buf, 64, "%.1f %s", downloaded, units);

	return buf;
}

IBittorrent* CreateIBittorrent()
{
	return new CBittorrent();
}

CBittorrent::CBittorrent(void)
{
}

CBittorrent::~CBittorrent(void)
{
}

static void AddTorrent(path file) {
	try {
		const path rfile=getmodulepath()/"resume"/file;

		libtorrent::entry metadata=bdecode(file);
		libtorrent::entry resumedata;

		if(boost::filesystem::exists(rfile)) {
			try {
				resumedata=bdecode(rfile.leaf());
			}
			catch(...) {
				char text[256], title[128];
				LoadString(GetModuleHandle(NULL), IDS_RESUMEERR, title, 128);
				LoadString(GetModuleHandle(NULL), IDS_RESUMEERRTEXT, text, 256);
				boost::filesystem::remove(rfile);
			}
		}

		if(!boost::filesystem::exists(getmodulepath()/"torrents"))
			boost::filesystem::create_directory(getmodulepath()/"torrents");

		if(!boost::filesystem::exists(getmodulepath()/"torrents"/file.leaf()))
			boost::filesystem::copy_file(file, getmodulepath()/"torrents"/file.leaf());

		if(!boost::filesystem::exists(conf.savepath))
			boost::filesystem::create_directory(conf.savepath);

		vector<libtorrent::torrent_handle>::size_type i=torrents.size();

		Torrent t;
		t.file=file.leaf();
		t.handle=session->add_torrent(metadata, conf.savepath, resumedata);

		t.handle.set_max_uploads(conf.maxup);
		t.handle.set_max_connections(conf.maxcon);

		const libtorrent::torrent_info &info=t.handle.get_torrent_info();

		t.cols[0]=info.name();
		t.cols[1]=strsize((double)info.total_size());

		torrents.push_back(t);
		sort(torrents.begin(), torrents.end());

		if(allpaused) t.handle.pause();
	}
	catch(exception &ex) {
		string text=ex.what();
		string caption=loadstring(IDS_EXCEPTION);
	}
}

bool CBittorrent::InitModule( IBittorrentNotify* notify )
{
	notify_ =notify;
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if(!conf.Load()) {
		conf.Save();
	}

	try {
		session=new libtorrent::session(libtorrent::fingerprint("AR", 1, 0, 0, 1), pair<int,int>(conf.firstport, conf.lastport));

		session->disable_extensions();
		session->enable_extension(libtorrent::peer_connection::extended_metadata_message);
		session->enable_extension(libtorrent::peer_connection::extended_peer_exchange_message);
		session->enable_extension(libtorrent::peer_connection::extended_listen_port_message);

		session->set_upload_rate_limit((conf.uplimit!=-1)?conf.uplimit*1024:-1);
		session->set_download_rate_limit((conf.downlimit!=-1)?conf.downlimit*1024:-1);
		session->set_max_connections(conf.maxcon);

#ifdef _DEBUG
		session->set_severity_level(libtorrent::alert::debug);
#else
		session->set_severity_level(libtorrent::alert::info);
#endif
	}
	catch(exception &ex) {
		return false;
	}

	paused=loadstring(IDS_PAUSED);
	queued=loadstring(IDS_QUEUED);
	checking=loadstring(IDS_CHECKING);
	connecting=loadstring(IDS_CONNECTING);
	downloading=loadstring(IDS_DOWNLOADING);
	seeding=loadstring(IDS_SEEDING);
	unknown=loadstring(IDS_UNKNOWN);
	bytes=loadstring(IDS_BYTES);
	kibibytes=loadstring(IDS_KIBIBYTES);
	mebibytes=loadstring(IDS_MEBIBYTES);
	gibibytes=loadstring(IDS_GIBIBYTES);

	path p=getmodulepath()/"torrents";

	//WIN32_FIND_DATAA finddata={0};
	//HANDLE find=FindFirstFileA((p/"*.torrent").native_file_string().c_str(), &finddata);
	//if(find!=INVALID_HANDLE_VALUE) {
	//	do AddTorrent(p/finddata.cFileName);
	//	while(FindNextFileA(find, &finddata));
	//	FindClose(find);
	//}

	return true;
}

void CBittorrent::ReleaseModule()
{
	path p=getmodulepath()/"resume";
	CreateDirectoryA(p.native_directory_string().c_str(), NULL);

	for(vector<Torrent>::size_type i=0; i<torrents.size(); i++) {
		torrents[i].handle.pause();
		libtorrent::entry e=torrents[i].handle.write_resume_data();

		bencode(p/torrents[i].file, e);
	}

	delete session;

	conf.Save();

	delete this;
}

DWORD CBittorrent::OpenSource( const char* szUrl, bool bSource )
{
	path file =szUrl;
	AddTorrent(file);
	return 0;
}

void CBittorrent::CloseSource( DWORD dwChannelID )
{
}

void CBittorrent::RequestSegment( DWORD dwChannelID, DWORD dwStartPos, DWORD dwLength )
{
}

bool CBittorrent::ReadSegment( DWORD dwChannelID, DWORD dwStartPos, char* pBuffer, DWORD& dwLength )
{
	return false;
}

LONGLONG CBittorrent::GetChannelSize( DWORD dwChannelID )
{
	return 0;
}

bool CBittorrent::GetChannelMonitorInfo( DWORD dwChannelID, stMonitorInfo& monInfo)
{
	return false;
}

bool CBittorrent::GetChannelMonitorInfo( const char* szChannelHash, stMonitorInfo& monInfo)
{
	return false;
}

bool CBittorrent::GetAllChannelID( list<DWORD>& listChannels)
{
	return false;
}

void CBittorrent::tick_it()
{
	static DWORD count = 0;
	static DWORD ts = GetTickCount();
	if ( ts + 1000 > GetTickCount())
		return;
	ts = GetTickCount();
	count++;

	char buf[32];
	libtorrent::session_status s=session->status();

	for(vector<Torrent>::size_type i=0; i<torrents.size(); i++) {
		try {
			libtorrent::torrent_status status=torrents[i].handle.status();

			torrents[i].cols[2]=strsize((double)status.total_done);
			torrents[i].cols[3]=strsize((double)status.total_upload);

			if(status.paused) torrents[i].cols[4]=paused;
			else switch(status.state) {
					case libtorrent::torrent_status::queued_for_checking:
						torrents[i].cols[4]=queued;
						break;
					case libtorrent::torrent_status::checking_files:
						torrents[i].cols[4]=checking;
						break;
					case libtorrent::torrent_status::connecting_to_tracker:
						torrents[i].cols[4]=connecting;
						break;
					case libtorrent::torrent_status::downloading:
					case libtorrent::torrent_status::downloading_metadata:
						torrents[i].cols[4]=downloading;
						break;
					case libtorrent::torrent_status::seeding:
						torrents[i].cols[4]=seeding;
						break;
					default:
						torrents[i].cols[4]=unknown;
						break;
			}

			StringCchPrintf(buf, 32, "%.1f%%", (double)status.progress*100.0);
			torrents[i].cols[5]=buf;

			StringCchPrintf(buf, 32, "%s/s", strsize(status.download_rate));
			torrents[i].cols[6]=buf;

			StringCchPrintf(buf, 32, "%s/s", strsize(status.upload_rate));
			torrents[i].cols[7]=buf;

			StringCchPrintf(buf, 32, "%d%%", (int)(status.distributed_copies*100.0f));
			torrents[i].cols[8]=buf;

			torrents[i].cols[9]=_itoa(status.num_seeds, buf, 10);
			torrents[i].cols[10]=_itoa(status.num_peers, buf, 10);

			////////////////////////////////////////////////////////////
			/// Process alerts

			bool bprintf =false;
			for(auto_ptr<libtorrent::alert> a=session->pop_alert(); a.get(); a=session->pop_alert()) {
				string timestamp=to_simple_string(a->timestamp().time_of_day());
				string message=a->msg();
				printf("%s : %s\n", timestamp.c_str(), message.c_str());
				bprintf = true;
			}

			if ( count % 5 == 0)
			{
				for( int j = 0; j < columncount; j++)
				{
					char buf[1024];
					int len=LoadString(GetModuleHandle(NULL), columns[j], buf, 1024);
					printf( "%s:%s\n", buf, torrents[i].cols[j].c_str());
				}

				StringCchPrintf(buf, 32, "D:%s/s", strsize(s.download_rate));
				printf("%s,", buf);

				StringCchPrintf(buf, 32, "U:%s/s", strsize(s.upload_rate));
				printf("%s", buf);

				printf("\n\n");
			}

			//after one minute force get announce url
			if ( count % 60  == 0) 
				torrents[i].handle.force_reannounce();
		}
		catch(exception &ex) {
			string text=ex.what();
			printf("%s : %s", text.c_str(), "Exception");
			continue;
		}
	}
}