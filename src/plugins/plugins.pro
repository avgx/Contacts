TEMPLATE = subdirs

SUBDIRS += optionsmanager
SUBDIRS += xmppstreams
SUBDIRS += iqauth
SUBDIRS += saslauth
SUBDIRS += stanzaprocessor
SUBDIRS += roster
SUBDIRS += presence
SUBDIRS += rostersmodel
SUBDIRS += mainwindow
SUBDIRS += rostersview
SUBDIRS += statuschanger
SUBDIRS += rosterchanger
SUBDIRS += accountmanager
SUBDIRS += traymanager
SUBDIRS += privatestorage
SUBDIRS += messageprocessor
SUBDIRS += messagewidgets
SUBDIRS += messagestyles
SUBDIRS += adiummessagestyle
SUBDIRS += chatmessagehandler
SUBDIRS += compress
SUBDIRS += connectionmanager
SUBDIRS += defaultconnection
SUBDIRS += starttls
SUBDIRS += statusicons
SUBDIRS += emoticons
SUBDIRS += vcard
SUBDIRS += dataforms
SUBDIRS += servicediscovery
SUBDIRS += gateways
SUBDIRS += avatars
SUBDIRS += notifications
SUBDIRS += autostatus
SUBDIRS += rostersearch
SUBDIRS += console
SUBDIRS += chatstates
SUBDIRS += xmppuriqueries
win32: {
SUBDIRS += stylesheeteditor
}
SUBDIRS += bitsofbinary
SUBDIRS += masssendhandler
SUBDIRS += registration
SUBDIRS += ramblerhistory
SUBDIRS += birthdayreminder
SUBDIRS += metacontacts
win32-msvc2008: {
SUBDIRS += sipphone
}
SUBDIRS += smsmessagehandler
