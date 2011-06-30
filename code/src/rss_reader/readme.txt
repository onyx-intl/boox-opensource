NewsFlash Version 0.92 (beta)

Into
°°°°
NewsFlash is a RSS aggregator and downloader for the Onyx Boox 60 and Bebook Neo. It allows you to read your favourite RSS feeds offline. NewsFlash automatically downloads the linked HTML page and tries to remove the navigation so you only have the information left. It downloads all the referenced images to your boox, so you can read even feeds offline that only supply a summery within the RSS information.


Installation
°°°°°°°°°°°°
Simply copy the NewsFlash.oar and NewsFlash.ini int a folder on your boox. You can name the folder any name you like. NewsFlash will autodetect where it was copied and create a Cache folder within the same directory.


Usage
°°°°°
To add a new rss feed to the NewsFlash.ini you have to create a new section within the config file. Sections are created by inserting a section header in brackets []. The name between the brackets is shown as the feed name. It does not have to correspond with the real name of the feed. Use what ever suites your needs.

Warning: Currently if you remove a feed from the config file the cache entry will not be deleted. Simply delete the cache directory if you deleted a news feed from the config file.

Within the section there is only one parameter that must be specified. The url-parameter has to be set to the download url of the XML feed data. NewsFlash understands RSS and Atom feeds.

There are some more parameters that can be set, if neccessary:

- keep
  Specifies the number of days you want to keep the feed items. The default
  is 3.

- hint
  This is an advanced setting. It defines a hint to the content analyser. If the
  downloaded page is missing some parts, you can set a hint (a class, style or
  name of a div-tag) where the content is located within the page.

- threshold
  Defines the threshold for the content parser. See the details section for a
  description of this parameter. Normaly you don't have to use it.

- fetchlink
  Can be used to disable fetching the linked page. This can be usefull to speed
  up updates if all information is already included within the RSS feed.
  Set fetchlink to 0 to disable fetching linked page. The default is 1.

- allimages
  Can be used to enable downloading images from external URLs. Normally if
  you're fetching test.com and the image is downloaded from asdf.com this image
  will be skiped. This speeds up downloading if there are many ads on a page.
  If you're missing some images, set this value to 1.
  The default is 0.

To download you daily news flash, simply launch NewsFlash.oar, switch on Wifi and press the Menu-Key and select the "Update feeds" menu group and click on "Select Wifi" to start downloading.the feeds. After the download has finished, you can open the feed by taping the feed item. In the right corner of the feed item the total amount of items and the new items (new/total) are shown.

You can cancel downloading by pressing the back-button while the update is running. The update will be stoped as soon as possible.

If you have more feeds than what fit on the screen you can scroll the list by tapping and draging the list up or down.

To select a feed (the black triangle in front of the title) for usage with the menu items in the "Feed" group simply tap on the feed an pull right. This selects the feed without opening it. Now you can press the menu key to select the "Feed" group and delete, clear or mark the feed.

You can mark a feed (for later reading, etc.) with a star via the menu. To quickly mark a feed you can tap on it and pull the stylus to the left.

To navigate the feed list without the stylus use the silver ring. Up and down selects the previous/next feed. Left an right jumps to the previus/next unread news feed.

To exit the application press the back button or open the menu and choose the exit icon.


Reading a feed without the stylus
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
While reading a feed you can use the buttons on the boox to navigate the items. Use the left and right keys on the silver ring to select the item to read (it will have a filled bullet infront of the date). Press the center Ok button to open the item. To go back to the item selection press the Ok button again. Scroll up and down with the Prev and Next keys.


Details
°°°°°°°
The NewsFlash application first of all downloads and parses the RSS or Atom information. It selects the feed items that are within the time period specified in the "keep" config option. It also loads the already downloaded feed information from the cache. All feed items that are too old will be removed from the cache. Then NewsFlash opens the linked page for each RSS item and downloads it. It then searches all DIV- and SPAN-sections within the file to detect the area where the main content is stored. This is done by comparing the plain text (without links, etc.) of the area with the full text. If this percentage is greater as the threshold, the range will be added to the output page. The default percentage is 35%. You can change this value with the threshold option for each feed.
After that, the remaining content is parsed and all IMG-tags will be downloaded into the cache. The references within the page will automatically be replaced with the downloaded images.


Known Bugs and Limitations
°°°°°°°°°°°°°°°°°°°°°°°°°°
- Some feeds hostes by microsoft can not be downloaded because they
  use JavaScript based redirection. This is a minor issue because
  these feeds supply all information within the rss. You can use
  fetchlink=0 to speedup downloading these feeds.
- Sometimes after putting the boox into standby the icons on the menu
  are missing (but the buttons work).


Changelog
°°°°°°°°°
Version 0.5
  - Initial beta release

Version 0.6
  - Show an error message if the ini file is not found.
  - Search for the ini file in the same directory where the
    software is installed.
  - Create a cache directory where the software is installed.
  - Reset the unread counter to 0 if a feed is opened.
  - Timestamp for every feed item on the overview page.
  - Nicer layout of the overview page for every feed.
  - Fix for Atom-feeds storing the link destination inside a
    href attribute.
  - Introduced a 600kiB in memory cache for small downloaded feed
    items like images and spacer gifs. This should speed up
    feed updates.
  - Added the boox toolbar and a menu for exit and rotate screen.
  - Added the refresh button and the battery indicator.

Version 0.7
  - Menu Key now opens the menu.
  - Usage of the Wifi icon for updateing the feeds.
  - Better handling of Wifi dialog (fixes password loss problem).
  - Handling of <meta content-type> tag for UTF8 encoded pages.
    Solves problems with some cache pages.
  - Optimized progress bar to make progress more visible and reduce
    the illusion of hanging.
  - Downloading of images within the feeds summary.
  - Items updatet during the last update are markt with a new tag
    in overview.
  - Fetching the linked page can now be disabled with the fetchlink
    option.
  - The time displayd in the feed list was UTC not the local time.
  - Now you can select a feed with the silver navigation ring and
    open it by clicking the middle Ok button. (No stylus needed)

Version 0.8
  - The new-Flag and the new items count are now based on the saved
    timestamp of the last reading of the feed. This makes it
    possible to download new items without loosing the information
    what is new and what is old.
  - Scrolling of the feed lits by draging it with the stylus.
  - Clearing the cache of a feed via the main menu.
  - New main menu icons.
  - License button within the main menu.
  - Better visible selection mark (triangle on the left)
  - Gesture support for marking a feed. Simply tap on it and slide
    to the right (or left). This way you can mark a feed without
    opening it.

Version 0.9
  - Better error message on malformed feeds.
  - Fixed a crash with the WiFi dialog when updating the feed a
    second time (often after canceling).
  - Implemented keypad navigation for the feed items.
  - You can use the left and right key of the inner ring to select
    the next/prev unread feed.
  - Feed status is saved after update so it is retained in case of
    a later crash.
  - Possibility to mark a feed via the menu or via a gesture:
    Tap on the feed and move the stylus to the left.
  - Selecting a feed can now only been done by taping and pulling
    right. This is because the "pull left" gesture is used for
    marking.
  - Better screen update for firmware Version 1.4.


Version 0.92
  - Better Handling of some Pages that caused a hang or crash.
  - New "fast" flag to skip downloading images from external URLs.

Roadmap
°°°°°°°
Ok, forget the version numbers. I'll implement features whenever I've time and they fit into the flow...

[ ] Add a user configurable debug mode for determining the value for
    threshold and hint
[ ] Insert boox-menu for "add feed"
[ ] Removal of navigation divs from body (hiding).
