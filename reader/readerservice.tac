# Copyright 2011 Arjan Scherpenisse <arjan@scherpenisse.net>
# See LICENSE for details.

"""
Script which reads
"""


readerMap = {
    "A3T6PWTJ": "like",
    "A3T6PYXZ": "made",
    "A3T6PWYP": "clear"
    }


from zope.interface import implements

from twisted.application import service

from fizzjik.hub import Hub
from fizzjik.input import sonmicro
from fizzjik import interfaces, rfid

from twisted.internet import reactor
from twisted.internet.defer import succeed
from twisted.web.iweb import IBodyProducer

from twisted.web.client import Agent
from twisted.web.http_headers import Headers



class StringProducer(object):
    implements(IBodyProducer)

    def __init__(self, body):
        self.body = body
        self.length = len(body)

    def startProducing(self, consumer):
        consumer.write(self.body)
        return succeed(None)

    def pauseProducing(self):
        pass

    def stopProducing(self):
        pass




class TagReceiverService (service.Service):
    implements(interfaces.IController)

    def startService(self):
        self.agent = Agent(reactor)


    def registerObservers(self, hub):
        """
        Registration of event observers.
        """
        hub.addObserver(rfid.TagAddedEvent, self.on_tag_added)
        hub.addObserver(rfid.TagRemovedEvent, self.on_tag_removed)


    def on_tag_added(self, event):
        print "TAG ADDED:"
        print event.data
        s = event.service.serial
        if s in readerMap:
            id = readerMap[s]
        else:
            print "*** UNKNOWN READER! %s - using 'like'." % s
            id = "like"

        self.agent.request("POST", "http://wowserver:8080/tag/%s" % id, Headers({}), StringProducer(event.data))



    def on_tag_removed(self, event):
        print "TAG REMOVED:"
        print event.data
        self.agent.request("DELETE", "http://wowserver:8080/tag", Headers({}), StringProducer(event.data))




hub = Hub()

# Create dbus mainloop
import dbus.mainloop.glib
dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

# monitor
monitor = sonmicro.SonMicroMifareSensorMonitor()
monitor.setServiceParent(hub)

#
svc = TagReceiverService()
svc.setServiceParent(hub)


application = service.Application('wow rfid reader controller')
hub.setServiceParent(application)
