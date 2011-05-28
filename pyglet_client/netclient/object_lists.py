# generic game object datastore
# has dictionary interface for retrieving items
class GenericObjectList:

    def __init__(self):
        self.objects = {}
        self._metaname = 'GenericStateList'
        self._itemname = 'GenericObject'
        self._object_type = None

    def __getitem__(self, key):
        if not key in self.objects:
            print '%s: %s does not exist: id= %s' % (self._metaname, self._itemname, str(key),)
        return self.objects[key]
        
    def __setitem__(self, key, value):
        self.objects[key] = value
        
    def __delitem__(self, key):
        del self.objects[key]
        
    def __len__(self):
        return len(self.objects)

    def __contains__(self, key):
        return key in self.objects

    def __iter__(self):
        return iter(self.objects)

    def keys(self):
        return self.objects.keys()

    def values(self):
        return self.objects.values()

    def items(self):
        return self.objects.items()

    def _add(self, *args):
        print args
        object = self._object_type(*args)
        self.objects[object.id] = object
        print '%s: %s created; id= %s' % (self._metaname, self._itemname, object.id,)
        return object
        
    def _remove(self, obj):
        id = obj.id
        if id in self.objects:
            del self.objects[id]
            print '%s: %s removed; id= %s' % (self._metaname, self._itemname, id,)
            return True
        return False

# datastore for agents
class AgentList(GenericObjectList):

    def __init__(self):
        from agents import Agent
        GenericObjectList.__init__(self)
        self._metaname = 'AgentList'
        self._itemname = 'Agent'
        self._object_type = Agent

    def create(self, player_id, **agent):
        agent = self._add(player_id, **agent)
        return agent

    def destroy(self, agent):
        self._remove(agent)
        return agent

# datastore for Players
class PlayerList(GenericObjectList):

    def __init__(self):
        from players import Player
        GenericObjectList.__init__(self)
        self._metaname = 'PlayerList'
        self._itemname = 'Player'
        self._object_type = Player
        self.client_ids = {}
        self.names = {}

    def join(self, **player):
        client_id = player.get('cid', None)
        name = player.get('name', None)
        if client_id is None or name is None:
            print 'player cannot join: player missing client_id or name'
            print player
            return
        player = self._add(client_id, name, player)
        self.client_ids[client_id] = player.id
        self.names[name] = client_id
        return player

    def join_yourself(self):
        from players import Player, YouPlayer
        self._object_type = YouPlayer
        player = self._add()
        self._object_type = Player
        self.client_ids[player.cid] = player.id
        self.names[player.name] = player.cid
        return player
        
    def leave(self, player):
        client_id = player.cid
        if self._remove(player) and client_id in self.client_ids:
            del self.client_ids[client_id]
        return player

    def by_name(self, name):    # returns a client_id
        if name in self.names:
            return self.names[name]
        else:
            return 0

    def update(self, player, id=None):
        if id is not None:
            if player.you and id not in self:
                id = 0
            old = self[id]
        else:
            old = player
        if old.cid in self.client_ids:
            del self.client_ids[old.cid]
        if old.name in self.names:
            del self.names[old.name]
        self.client_ids[player.cid] = player.id
        self.names[player.name] = player.cid
