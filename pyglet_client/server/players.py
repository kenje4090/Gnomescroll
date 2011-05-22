from game_state import GenericObjectList
from game_state import GameStateGlobal

# datastore for Players
class PlayerList(GenericObjectList):

    def __init__(self):
        GenericObjectList.__init__(self)
        self._metaname = 'PlayerList'
        self._itemname = 'Player'
        self._object_type = Player
        self.client_ids = {}

    def join(self, client_id, name):
        player = self._add(client_id, name)
        self.client_ids[client_id] = player.id
        
    def leave(self, player):
        client_id = player.client_id
        if self._remove(player) and client_id in self.client_ids:
            del self.client_ids[client_id]
        
                
# represents a "Player" (player score, agents they control etc)
class Player:

    def __init__(self, client_id, name, id=None):
        self.kills = 0
        self.deaths = 0
        self.name = name
        self.client_id = client_id
        if id is None:
            id = GameStateGlobal.new_player_id()
        self.id = id

    def killed(self):
        self.kills += 1

    def died(self):
        self.deaths += 1

    def quit(self):
        GameStateGlobal.playerList.leave(self)
