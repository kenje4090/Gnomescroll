import opts
opts = opts.opts

import math

import c_lib.terrain_map as terrain_map
import c_lib.c_lib_agents as cAgents

class GameStateGlobal:

    gameState = None
    terrainMap = None
    agentList = None
    playerList = None
    weaponList = None
    #teamList = None
    #game = None

    player_id = 0

    fps = 30.
    agent_tick_mode = 'jetpack'

    def __init__(self):
        GameStateGlobal.terrainMap = terrain_map
        GameStateGlobal.gameState = GameState()
        GameStateGlobal.agentList = AgentList()
        GameStateGlobal.playerList = PlayerList()
        GameStateGlobal.weaponList = WeaponList()
        #GameStateGlobal.teamList = game_modes.TeamList()

        #GameStateGlobal.game_mode_name = opts.game_mode
        #game_args = {
            #'teams' :   opts.n_teams,
            #'team_kills':   opts.team_kills,
        #}
        #game = game_modes.names[opts.game_mode]
        #GameStateGlobal.game = game(**game_args)

    @classmethod
    def init(cls):
        pass

    @classmethod
    def new_player_id(cls):
        cls.player_id += 1
        return cls.player_id

    @classmethod
    def disconnect(self, connection):
        if connection.player is not None:
            if connection.player.agent is not None:
                connection.player.agent.quit()
                GameStateGlobal.agentList.destroy(connection.player.agent)
            connection.player.quit()
            #GameStateGlobal.game.remove_player(connection.player)

from net_server import NetServer

from agents import AgentList
from players import PlayerList
from weapons import WeaponList
#import game_modes

# main game state wrapper
class GameState:

    def __init__(self):
        self.time = 0

    # tick all agents
    def tick(self):
        for agent in GameStateGlobal.agentList.values():
            agent.tick()
        self.time += 1
        if self.time % 100 == 0:
            print "time= %i" % (self.time,)
