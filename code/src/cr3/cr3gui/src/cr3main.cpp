/*
    First version of CR3 for EWL, based on etimetool example by Lunohod
*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
//#include <Ewl.h>
#include <crengine.h>
#include <crgui.h>
#include <crtrace.h>

#include "cr3main.h"
#include "mainwnd.h"

bool loadKeymaps( CRGUIWindowManager & winman, const char * locations[] )
{
	bool res = false;
	for ( int i=0; locations[i]; i++ ) {
		lString8 location( locations[i] );
		char lastChar = location[ location.length() - 1 ];
		if ( lastChar!='/' && lastChar!='\\' )
#if defined(_WIN32) && !defined(__WINE__)
			location << "\\";
#else
			location << "/";
#endif
		lString8 def = location + "keydefs.ini";
		lString8 map = location + "keymaps.ini";
		lString8 layout = location + "kblayout.ini";
		winman.getKeyboardLayouts().openFromFile( layout.c_str() );
		CRGUIAcceleratorTableList tables;

		if ( tables.openFromFile(  def.c_str(), map.c_str() ) ) {
			res = true;
			winman.getAccTables().addAll( tables );
		}
	}
    if ( winman.getAccTables().empty() ) {
        CRLog::error("keymap files keydefs.ini and keymaps.ini were not found! please place them to ~/.crengine or /etc/cr3");
    }
#if 0
    static const int menu_acc_table[] = {
        XK_Escape, 0, MCMD_CANCEL, 0,
        XK_Return, 0, MCMD_OK, 0, 
        XK_Return, 1, MCMD_OK, 0, 
        '0', 0, MCMD_SCROLL_FORWARD, 0,
        XK_Down, 0, MCMD_SCROLL_FORWARD, 0,
        '9', 0, MCMD_SCROLL_BACK, 0,
        XK_Up, 0, MCMD_SCROLL_BACK, 0,
        '0', 1, MCMD_LONG_FORWARD, 0,
        XK_Down, 1, MCMD_LONG_FORWARD, 0,
        '9', 1, MCMD_LONG_BACK, 0,
        XK_Up, 1, MCMD_LONG_BACK, 0,
        '1', 0, MCMD_SELECT_1, 0,
        '2', 0, MCMD_SELECT_2, 0,
        '3', 0, MCMD_SELECT_3, 0,
        '4', 0, MCMD_SELECT_4, 0,
        '5', 0, MCMD_SELECT_5, 0,
        '6', 0, MCMD_SELECT_6, 0,
        '7', 0, MCMD_SELECT_7, 0,
        '8', 0, MCMD_SELECT_8, 0,
        0
    };
    if ( winman.getAccTables().get("menu").isNull() )
        winman.getAccTables().add("menu", menu_acc_table );
    static const int acc_table_dialog[] = {
        XK_Escape, 0, MCMD_CANCEL, 0,
        XK_Return, 1, MCMD_OK, 0, 
        XK_Return, 0, MCMD_OK, 0, 
        XK_Down, 0, MCMD_SCROLL_FORWARD, 0,
        XK_Up, 0, MCMD_SCROLL_BACK, 0,
        '0', 0, MCMD_SELECT_0, 0,
        '1', 0, MCMD_SELECT_1, 0,
        '2', 0, MCMD_SELECT_2, 0,
        '3', 0, MCMD_SELECT_3, 0,
        '4', 0, MCMD_SELECT_4, 0,
        '5', 0, MCMD_SELECT_5, 0,
        '6', 0, MCMD_SELECT_6, 0,
        '7', 0, MCMD_SELECT_7, 0,
        '8', 0, MCMD_SELECT_8, 0,
        '9', 0, MCMD_SELECT_9, 0,
        0
    };
    if ( winman.getAccTables().get("dialog").isNull() )
        winman.getAccTables().add("dialog", acc_table_dialog );
    static const int default_acc_table[] = {
        '6', 0, MCMD_GO_LINK, 0,
        '8', 0, MCMD_SETTINGS_FONTSIZE, 0,
        '8', 1, MCMD_SETTINGS_ORIENTATION, 0,
        XK_Escape, 0, MCMD_QUIT, 0,
        XK_Return, 0, MCMD_MAIN_MENU, 0,
        XK_Return, 1, MCMD_SETTINGS, 0,
        '0', 0, DCMD_PAGEDOWN, 0,
        XK_Up, 0, DCMD_PAGEDOWN, 0,
        '0', KEY_FLAG_LONG_PRESS, DCMD_PAGEDOWN, 10,
        XK_Up, KEY_FLAG_LONG_PRESS, DCMD_PAGEDOWN, 10,
        XK_Down, 0, DCMD_PAGEUP, 0,
        XK_Down, KEY_FLAG_LONG_PRESS, DCMD_PAGEUP, 10,
        '9', 0, DCMD_PAGEUP, 0,
        '9', KEY_FLAG_LONG_PRESS, DCMD_PAGEUP, 10,
#ifdef WITH_DICT
        '2', 0, MCMD_DICT, 0,
#endif
        '+', 0, DCMD_ZOOM_IN, 0,
        '=', 0, DCMD_ZOOM_IN, 0,
        '-', 0, DCMD_ZOOM_OUT, 0,
        '_', 0, DCMD_ZOOM_OUT, 0,
        0
    };
    if ( winman.getAccTables().get("main").isNull() )
        winman.getAccTables().add("main", default_acc_table );
#endif
	return res;
}

#if 0
bool initHyph(const char * fname)
{
    //HyphMan hyphman;
    //return;

    LVStreamRef stream = LVOpenFileStream( fname, LVOM_READ);
    if (!stream)
    {
        printf("Cannot load hyphenation file %s\n", fname);
        return false;
    }
    TexHyph * method = new TexHyph();
    if ( method->load( stream ) ) {
        _method = method;
        return true;
    }
    _method = NO_HYPH;
    return false;
}
#endif

lString8 readFileToString( const char * fname )
{
    lString8 buf;
    LVStreamRef stream = LVOpenFileStream(fname, LVOM_READ);
    if (!stream)
        return buf;
    int sz = stream->GetSize();
    if (sz>0)
    {
        buf.insert( 0, sz, ' ' );
        stream->Read( buf.modify(), sz, NULL );
    }
    return buf;
}

void ShutdownCREngine()
{
    HyphMan::uninit();
    ShutdownFontManager();
    CRLog::setLogger( NULL );
}

#if (USE_FREETYPE==1)
bool getDirectoryFonts( lString16Collection & pathList, lString16Collection & ext, lString16Collection & fonts, bool absPath )
{
    int foundCount = 0;
    lString16 path;
    for ( unsigned di=0; di<pathList.length();di++ ) {
        path = pathList[di];
        LVContainerRef dir = LVOpenDirectory(path.c_str());
        if ( !dir.isNull() ) {
            CRLog::trace("Checking directory %s", UnicodeToUtf8(path).c_str() );
            for ( int i=0; i < dir->GetObjectCount(); i++ ) {
                const LVContainerItemInfo * item = dir->GetObjectInfo(i);
                lString16 fileName = item->GetName();
                lString8 fn = UnicodeToLocal(fileName);
                    //printf(" test(%s) ", fn.c_str() );
                if ( !item->IsContainer() ) {
                    bool found = false;
                    lString16 lc = fileName;
                    lc.lowercase();
                    for ( int j=0; j<ext.length(); j++ ) {
                        if ( lc.endsWith(ext[j]) ) {
                            found = true;
                            break;
                        }
                    }
                    if ( !found )
                        continue;
                    lString16 fn;
                    if ( absPath ) {
                        fn = path;
                        if ( !fn.empty() && fn[fn.length()-1]!=PATH_SEPARATOR_CHAR)
                            fn << PATH_SEPARATOR_CHAR;
                    }
                    fn << fileName;
                    foundCount++;
                    fonts.add( fn );
                }
            }
        }
    }
    return foundCount > 0;
}
#endif

bool InitCREngine( const char * exename, lString16Collection & fontDirs )
{
    CRLog::trace("InitCREngine(%s)", exename);
    for ( int k=0; k<fontDirs.length(); k++ )
        CRLog::trace(" fontDir: %s", LCSTR(fontDirs[k]));
    lString16 appname( exename );
    int lastSlash=-1;
    lChar16 slashChar = '/';
    for ( int p=0; p<(int)appname.length(); p++ ) {
        if ( appname[p]=='\\' ) {
            slashChar = '\\';
            lastSlash = p;
        } else if ( appname[p]=='/' ) {
            slashChar = '/';
            lastSlash=p;
        }
    }

    lString16 appPath;
    if ( lastSlash>=0 )
        appPath = appname.substr( 0, lastSlash+1 );

    lString16 fontDir = appPath + L"fonts";
    fontDir << slashChar;
    lString8 fontDir8 = UnicodeToLocal(fontDir);
    //const char * fontDir8s = fontDir8.c_str();
    //InitFontManager( fontDir8 );
    InitFontManager( lString8() );

    // Load font definitions into font manager
    // fonts are in files font1.lbf, font2.lbf, ... font32.lbf
    if (!fontMan->GetFontCount()) {

    lString16Collection fontExt;
    #if (USE_FREETYPE==1)
        fontExt.add(lString16(L".ttf"));
        fontExt.add(lString16(L".otf"));
        fontExt.add(lString16(L".pfa"));
        fontExt.add(lString16(L".pfb"));
    #else
        fontExt.add(lString16(L".lbf"));
    #endif
    #if (USE_FREETYPE==1)
        lString16Collection fonts;
        fontDirs.add( fontDir );
        static const char * msfonts[] = {
            "arial.ttf", "arialbd.ttf", "ariali.ttf", "arialbi.ttf",
            "cour.ttf", "courbd.ttf", "couri.ttf", "courbi.ttf",
            "times.ttf", "timesbd.ttf", "timesi.ttf", "timesbi.ttf",
            NULL
        };
    #ifdef _LINUX
    #ifndef LBOOK
        fontDirs.add( lString16(L"/usr/local/share/crengine/fonts") );
        fontDirs.add( lString16(L"/usr/local/share/fonts/truetype/freefont") );
        fontDirs.add( lString16(L"/usr/share/crengine/fonts") );
        fontDirs.add( lString16(L"/usr/share/fonts/truetype/freefont") );
        fontDirs.add( lString16(L"/root/fonts/truetype") );
        fontDirs.add( lString16(L"/usr/share/fonts") );
        //fontDirs.add( lString16(L"/usr/share/fonts/truetype/msttcorefonts") );
        for ( int fi=0; msfonts[fi]; fi++ )
            fonts.add( lString16(L"/usr/share/fonts/truetype/msttcorefonts/") + lString16(msfonts[fi]) );
    #endif
    #endif
        getDirectoryFonts( fontDirs, fontExt, fonts, true );

        // load fonts from file
        CRLog::debug("%d font files found", fonts.length());
        //if (!fontMan->GetFontCount()) {
            for ( unsigned fi=0; fi<fonts.length(); fi++ ) {
                lString8 fn = UnicodeToLocal(fonts[fi]);
                CRLog::trace("loading font: %s", fn.c_str());
                if ( !fontMan->RegisterFont(fn) ) {
                    CRLog::trace("    failed\n");
                }
            }
        //}
    #else
            #define MAX_FONT_FILE 128
            for (int i=0; i<MAX_FONT_FILE; i++)
            {
                char fn[1024];
                sprintf( fn, "font%d.lbf", i );
                printf("try load font: %s\n", fn);
                fontMan->RegisterFont( lString8(fn) );
            }
    #endif
    }

    // init hyphenation manager
    //char hyphfn[1024];
    //sprintf(hyphfn, "Russian_EnUS_hyphen_(Alan).pdb" );
    //if ( !initHyph( (UnicodeToLocal(appPath) + hyphfn).c_str() ) ) {
#ifdef _LINUX
    //    initHyph( "/usr/share/crengine/hyph/Russian_EnUS_hyphen_(Alan).pdb" );
#endif
    //}

    if (!fontMan->GetFontCount())
    {
        //error
#if (USE_FREETYPE==1)
        printf("Fatal Error: Cannot open font file(s) .ttf \nCannot work without font\n" );
#else
        printf("Fatal Error: Cannot open font file(s) font#.lbf \nCannot work without font\nUse FontConv utility to generate .lbf fonts from TTF\n" );
#endif
        return false;
    }

    printf("%d fonts loaded.\n", fontMan->GetFontCount());

    return true;

}

void InitCREngineLog( const char * cfgfile )
{
    if ( !cfgfile ) {
        //CRLog::setStdoutLogger();
        //CRLog::setLogLevel( CRLog::LL_TRACE );
        return;
    }
    lString16 logfname(
#ifdef __arm__
                                               "/dev/null"
#else
                                               "stdout"
#endif
            );
    lString16 loglevelstr = L"INFO";
	bool autoFlush = false;
    CRPropRef logprops = LVCreatePropsContainer();
    {
        LVStreamRef cfg = LVOpenFileStream( cfgfile, LVOM_READ );
        if ( !cfg.isNull() ) {
            logprops->loadFromStream( cfg.get() );
            logfname = logprops->getStringDef( PROP_LOG_FILENAME,
#ifdef __arm__
                                               "/dev/null"
#else
                                               "stdout"
#endif
                                               );
            loglevelstr = logprops->getStringDef( PROP_LOG_LEVEL,
#ifdef __arm__
                                                  "INFO"
#else
                                                  "TRACE"
#endif
                                                  );
			autoFlush = logprops->getBoolDef( PROP_LOG_AUTOFLUSH, false );
        }
    }
    CRLog::log_level level = CRLog::LL_INFO;
    if ( loglevelstr==L"OFF" ) {
        level = CRLog::LL_FATAL;
        logfname.clear();
    } else if ( loglevelstr==L"FATAL" ) {
        level = CRLog::LL_FATAL;
    } else if ( loglevelstr==L"ERROR" ) {
        level = CRLog::LL_ERROR;
    } else if ( loglevelstr==L"WARN" ) {
        level = CRLog::LL_WARN;
    } else if ( loglevelstr==L"INFO" ) {
        level = CRLog::LL_INFO;
    } else if ( loglevelstr==L"DEBUG" ) {
        level = CRLog::LL_DEBUG;
    } else if ( loglevelstr==L"TRACE" ) {
        level = CRLog::LL_TRACE;
    }
    if ( !logfname.empty() ) {
        if ( logfname==L"stdout" )
            CRLog::setStdoutLogger();
        else if ( logfname==L"stderr" )
            CRLog::setStderrLogger();
        else
            CRLog::setFileLogger( UnicodeToUtf8( logfname ).c_str(), autoFlush );
    }
    CRLog::setLogLevel( level );
    CRLog::trace("Log initialization done.");
}

