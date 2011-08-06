'''
Toys, miscellaneous objects
'''

from game_objects import DetachableObject, StaticObject, TeamItem
from vox import GameObjectRender, TeamItemRender

class Flag(DetachableObject, TeamItem, TeamItemRender):

    def __init__(self, id, radius=None, team=None, state=None, **kwargs):
        if team is not None:
            team = GameStateGlobal.teamList[team]
        DetachableObject.__init__(self, id, radius, state=state)
        TeamItem.__init__(self, team, own=False, other=True)
        TeamItemRender.__init__(self, model='black_flag.vox')
        self.auto_grab = True
        self.drop_on_death = True
        self.type = 1
        self.radius = 1

    def take(self, new_owner):
        print new_owner.team, self.team
        if (new_owner.team == self.team and self.pickup_by_own_team) or \
            (new_owner.team != self.team and self.pickup_by_other_team):
            DetachableObject.take(self, new_owner)


class Base(StaticObject, GameObjectRender, TeamItem):

    def __init__(self, id, radius=None, team=None, state=None, **kwargs):
        if team is not None:
            team = GameStateGlobal.teamList[team]
        StaticObject.__init__(self, id, state)
        TeamItem.__init__(self, team, False, False)
        GameObjectRender.__init__(self, 'auto.vox')
        self.type = 2
        self.radius = 1

from game_state import GameStateGlobal
