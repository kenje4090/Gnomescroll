#!/usr/bin/python

'''
Client network outgoing
'''

class NetOut:
    sendPacket = None

    sendMessage = None
    mapMessage = None
    adminMessage = None
    chatMessage = None

    @classmethod
    def init_0(self):
        NetOut.sendPacket = NetClientGlobal.sendPacket
        NetOut.sendMessage = SendMessage()
        NetOut.mapMessage = MapMessage()
        NetOut.adminMessage = AdminMessage()
        NetOut.chatMessage = ChatMessage()
    @classmethod
    def init_1(self):
        assert NetOut.mapMessage != None
    @classmethod
    def send_json(self, dict):
        self.sendPacket.send_json(dict)
    @classmethod
    def send_binary(self, msg_id, bin_sting):
        self.sendPacket.send_binary(msg_id, bin_string)

from net_client import NetClientGlobal

class SendMessage:

    def send_agent_control_state(self, id, d_x, d_y, d_xa, d_za, jetpack, brake):
        d = {
            'cmd' : 'agent_control_state',
            'id' : id,
            'tick' : 0,
            'state': [d_x, d_y, d_xa, d_za, jetpack, brake]
           }
        NetOut.send_json(d)

    def send_client_id(self):
        d = {
            'cmd' : 'send_client_id',
            'id' : NetClientGlobal.client_id,
           }
        NetOut.send_json(d)

class MapMessage:

    def request_chunk_list(self):
        d = {
            'cmd' : 'request_chunk_list',
            }
        NetOut.send_json(d)

    def request_chunk(self, x,y,z):
        d = {
            'cmd' : 'request_chunk',
            'value' : (x,y,z)
        }

class ChatMessage:

    def send_chat(self, d):
        d['cmd'] = 'chat'
        d['client_id'] = str(NetClientGlobal.client_id)
        NetOut.send_json(d)

    def subscribe(self, channel):
        d = {
            'channel'   : channel,
            'cmd'       : 'subscribe',
            'client_id' : str(NetClientGlobal.client_id),
        }
        NetOut.send_json(d)

    def unsubscribe(self, channel):
        d = {
            'channel'   : channel,
            'cmd'       : 'unsubscribe',
            'client_id' : str(NetClientGlobal.client_id),
        }
        NetOut.send_json(d)

class AdminMessage:

    def set_map(self,x,y,z,value):
        d = {
            'set_map' : 'set_map',
            'list' : [(x,y,z,value)],
            }
        NetOut.send_json(d)

    def set_map_bulk(self, list): #takes a list of 4 tuples of (x,y,z,value)
        d = {
            'set_map' : 'set_map',
            'list' : list,
            }
        NetOut.send_json(d)
