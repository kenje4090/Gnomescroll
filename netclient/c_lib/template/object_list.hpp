#pragma once 



template <class Object_state, int max_n=1024>
class Object_list {
        private:
                int id_c;
                static const char* name() { return "Object"; }
        public:
                int num;
                Object_state* a[max_n];

                Object_list(); //default constructor
                Object_state* get(int id);
                Object_state* create();			//object auto id
                Object_state* create(int id);	//create object with id
                void destroy(int _id);

                void draw();	//overide in template specilization on client
                void tick();	//override on template specilization
        };


//template <class T>
//Stack<T>::Stack(int s)
template <class Object_state, int max_n>
Object_list<Object_state, max_n>::Object_list() 
{
	num = 0;
	id_c = 0;
	int i;
	for(i=0;i<max_n;i++) a[i] = NULL;
}

template <class Object_state, int max_n>
Object_state* Object_list<Object_state, max_n>::get(int id)
{
	if((id < 0) || (id >= max_n)) {
		printf("%s id error: id=%i\n", name ,id);
		return NULL;
	} 
	if(a[id] == NULL) {
		printf("%s get error: object is null, id=%i\n",name, id);
		return NULL;
	}
	return a[id];
}

template <class Object_state, int max_n>
Object_state* Object_list<Object_state, max_n>::create() {
		int i;
		int id = id_c;
		id_c++;
		for(i=0; i<max_n;i++) {
			id = (i+id)%max_n;
			if(a[id] == NULL) break;
		}
		if(i==1024) {
			printf("%s_list Error: cannot create object, object limit exceeded\n", name);
			return NULL;
		}
		num++;
		a[id] = new Object_state(id);
		printf("%s_list: Created object %i\n", name, id);
}

template <class Object_state, int max_n>
Object_state* Object_list<Object_state, max_n>::create(int id) {
	if(a[id] == NULL) {
		a[id] = new Object_state(id);
		printf("%s_list: Created object from id: %i\n", name, id);
		return a[id];
	} else {
		printf("%s_list: Cannot Create object from id; id is in use: %i\n", name, id);
		return NULL;
	}
}


template <class Object_state, int max_n>
void Object_list<Object_state, max_n>::destroy(int id) {
	if(a[id]==NULL) {
		printf("%s_list: Cannot delete object: object is null\n", name);
		return;
	}
	delete a[id];
	a[id] = NULL;
	num--;
	printf("%s_list: Deleted object %i\n",name, id);
	//printf("Object_list::delete_agent not implemented\n");
}


template <class Object_state, int max_n>
void Object_list<Object_state, max_n>::draw() {
	
	printf("%s_list_list::draw, not implemented\n", name);

	#ifdef DC_CLIENT
	//AgentDraw::draw_agents(this);
	#endif
}


template <class Object_state, int max_n>
void Object_list<Object_state, max_n>::tick() {
	printf("%s_list::tick, not implemented\n", name);
}