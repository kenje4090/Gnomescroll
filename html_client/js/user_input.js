var input,
    keymap;

input = {

    queue: [],  // queue of events received
    
    init: function () { // init $ bindings
        
        $('body').keydown(function (event) {
            var key = keymap[event.which];
            input.queue.push({key: key, timestamp: event.timeStamp});
               }).click(function (event) {
                  var key = keymap[event.which];
                  if (key === 'left-click') { 
                      map_editor.set_tile(event);
                  } 
               });
    },
        
    next: function (delay) { // shifts the queue, FIFO
        var timestamp = 0,
            delay = delay || 300, // millisecond delay between input
            queue = this.queue;
        
        while (queue.length > 0) {
            var event = queue.shift();
            
            if (event === undefined || !(event.timestamp - timestamp) > delay) {
                continue;
            }
            
            timestamp = event.timestamp;
            queue = [];
            
            return event.key;
        }
        return false;
    }
};

// for the keydown event.
// NOTE: keypress keymap is COMPLETELY DIFFERENT FOR NO REASON
keymap = {
    1: 'left-click',
    
    81: 'q',
    87: 'w',
    69: 'e',
    82: 'r',
    84: 't',
    89: 'y',
    85: 'u',
    73: 'i',
    79: 'o',
    80: 'p',
    65: 'a',
    83: 's',
    68: 'd',
    70: 'f',
    71: 'g',
    72: 'h',
    74: 'j',
    75: 'k',
    76: 'l',
    90: 'z',
    88: 'x',
    67: 'c',
    86: 'v',
    66: 'b',
    78: 'n',
    77: 'm',
    
    49: 1,
    50: 2,
    51: 3,
    52: 4,
    53: 5,
    54: 6,
    55: 7,
    56: 8,
    57: 9,
    48: 0,
    
    27: 'ESC',
    13: 'ENTER',
    16: 'SHIFT',
    20: 'CAPS',
    18: 'ALT',
    17: 'CTRL',
    91: 'WIN',
    9:  'TAB',
    8:  'BACK',
    32: 'SPACE',
    
    37: 'LEFT',
    38: 'UP',
    39: 'RIGHT',
    40: 'DOWN',
    
    192: '~',
    189: '-',
    187: '=',
    219: '[',
    221: ']',
    220: '\\',
    186: ':',
    222: '\'',
    188: ',',
    190: '.',
    191: '/',
    
    36: 'HOME',
    35: 'END',
    46: 'DELETE',
    33: 'PAGEUP',
    34: 'PAGEDOWN',
    
    45: 'INSERT',
    19: 'PAUSEBREAK',
  //??: 'PRINTSCREEN', // intercepted by system
  
    146: 'NUMLOCK',
    111: '/NUMPAD',
    106: '*NUMPAD',
    109: '-NUMPAD',
    107: '+NUMPAD',
    
    /* these are completely different depending on numlock y/n
    35: '1NUMPAD',
    40: '2NUMPAD',
    34: '3NUMPAD',
    37: '4NUMPAD',
    39: '6NUMPAD',
    36: '7NUMPAD',
    38: '8NUMPAD',
    33: '9NUMPAD',
    */
    
    13: 'ENTERNUMPAD',
    
    112: 'F1',
    113: 'F2',
    114: 'F3',
    115: 'F4',
    116: 'F5',
    117: 'F6',
    118: 'F7',
    119: 'F8',
    120: 'F9',
    121: 'F10',
    122: 'F11',
    123: 'F12',
    
    // problematic (all map to 0?):
    //0: '5NUMPAD',
    //0: '.NUMPAD',
    //0: '0NUMPAD',
};

var processInput = function (key) {
    
    if (!key) return false;
    
    var old;
    
    switch (key) {
        
        case 'ESC':               //quit ? remove this
            map_editor.clear_current();
            return false;
            
        case 'LEFT':
            board.scroll(-1, 0);
            break;
            
        case 'RIGHT':
            board.scroll(1, 0);
            break;
            
        case 'UP':
            board.scroll(0, -1);
            break;
            
        case 'DOWN':
            board.scroll(0, 1);
            break;
            
        case 'c':                   // set map solid
            console.log('set map');
            //admin.set_map(selected_agent.pos(), 1);
            admin.set_map(cursor.pos(), 1);
            break;
            
        case 'x':                   // set map empty
            //admin.set_map(selected_agent.pos(), 0);
            admin.set_map(cursor.pos(), 0);
            break;
            
        case 'p':                   // create agent
            admin.create_agent(cursor.pos());
            break;
            
        case 't':
            admin.create_object(cursor.pos());
            break;
            
        case 'g':
            //admin.create_object(selected_agent.pos(), 
            break;
            
        case 'r':
            action.plant(selected_agent.id);
            break;
            
        case 'f':
            action.till(selected_agent.id);
            break;
            
        case 'v':
            action.harvest(selected_agent.id);
            break;
            
        case 'i':
            info.map(5);
            break;
    }
    return true;
};
