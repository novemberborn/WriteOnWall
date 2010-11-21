#
# Example how to use a sonmicro reader from twisted+python.
#
# Arjan Scherpenisse <arjan@scherpenisse.net>
#

import sys

from zope.interface import implements

from twisted.application import service, app
from twisted.internet import reactor
from twisted.web.client import Agent
from twisted.web.http_headers import Headers

from fizzjik.hub import Hub
from fizzjik import rfid, interfaces

from twisted.internet.defer import succeed
from twisted.web.iweb import IBodyProducer

from twisted.python import usage, log


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

    def registerObservers(self, hub):
        """
        Registration of event observers.
        """
        hub.addObserver(rfid.TagAddedEvent, self.on_tag_added)
        hub.addObserver(rfid.TagRemovedEvent, self.on_tag_removed)


    def on_tag_added(self, event):
        print "TAG ADDED:"
        print event.data
        agent.request("POST", "http://wowserver:8080/tag/%s" % options['id'], Headers({}), StringProducer(event.data))

    def on_tag_removed(self, event):
        print "TAG REMOVED:"
        print event.data
        agent.request("DELETE", "http://wowserver:8080/tag" % options['id'], Headers({}), StringProducer(event.data))


class Options(usage.Options):
    optParameters = [
        ['device', 'd', '/dev/ttyUSB0', 'The device to listen to'],
        ['id', 'i', 'default', 'Identifier for the device']
    ]

application = service.Application("sonmicro")

hub = Hub()
hub.setServiceParent(application)

svc = TagReceiverService()
svc.setServiceParent(hub)

agent = Agent(reactor)

from fizzjik.input.sonmicro  import SonMicroMifareSensor
rf = SonMicroMifareSensor()
options = Options()
options.parseOptions()
rf.device = options['device']
rf.setServiceParent(hub)

app.startApplication(application, False)
log.addObserver(log.FileLogObserver(sys.stdout).emit)
reactor.run()

