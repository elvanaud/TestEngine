#include <iostream>
#include <vector>
#include <conio.h>

const int EMPTY_COMPONENT = 0;
const int GRAPHIC_COMPONENT = 1;
const int NODE_COMPONENT = 2;

class Component
{
public:
    Component(int id)
    {
        m_id = id;
    }
    virtual ~Component(){}

    int getType(){return m_id;}
private:
    int m_id;
};

class GraphicComponent : public Component
{
public:
    GraphicComponent() : Component(GRAPHIC_COMPONENT)
    {

    }

    GraphicComponent(char t) : GraphicComponent()
    {
        c=t;
    }
    char c;
};

class NodeComponent : public Component
{
public:
    NodeComponent(int x, int y) : Component(NODE_COMPONENT)
    {
        this->x=x;
        this->y=y;
    }

    int x, y;
};

class EmptyComponent : public Component
{
private:
    EmptyComponent() : Component(EMPTY_COMPONENT)
    {

    }
    static EmptyComponent * me;
public:
    static const EmptyComponent &getInstance()
    {
        if(me == nullptr)
        {
            me = new EmptyComponent;
        }

        return *me;
    }
};

EmptyComponent * EmptyComponent::me = nullptr;

class Entity
{
public:
    enum Type{World, Player};
    Entity(Type t)
    {
        type = t;
    }

    void update(){}
    void render(){}
    void addComponent(Component & c)
    {
        m_components.push_back(&c);
    }

    const Component& getComponent(int id)
    {
        for(Component * c: m_components)
        {
            if(c->getType() == id)
                return *c;
        }

        return EmptyComponent::getInstance();
    }

private:
    Type type;
    std::vector<Component*> m_components;
};

class System
{
public:
    void registerEntity(Entity & e)
    {

    }

    void update(){}
    void addEntity(Entity & e)
    {
        m_entities.push_back(&e);
    }
protected:
    std::vector<Entity*> m_entities;
};

class GraphicSystem : public System
{
public:
    GraphicSystem()
    {
        refreshScene();
    }

    void refreshScene()
    {
        scene.erase(scene.begin(), scene.end());
        for(int i = 0; i < 10; i++)
        {
            scene.push_back(std::string(""));
        }
        for(std::string &l : scene)
        {
            l += "............................................";
        }
    }

    void registerEntity(Entity & e)
    {
        GraphicComponent g;
        e.addComponent(g);
        addEntity(e);
    }

    void update()
    {
        refreshScene();
        for(Entity *t : m_entities)
        {
            Entity e = *t;
            const GraphicComponent& g = (const GraphicComponent&) e.getComponent(GRAPHIC_COMPONENT);
            const NodeComponent &n = (const NodeComponent&)e.getComponent(NODE_COMPONENT); //BIG PROBLEM IF NO MATCHING COMPONENT

            scene[n.y][n.x] = g.c;

            render();
        }
    }

    void render()
    {
        system("cls");
        for(std::string & l : scene)
        {
            std::cout << l << std::endl;
        }
    }
private:
    std::vector<std::string> scene;
};

class PlayingSystem : public System
{
public:
    void update()
    {
        char c = getch();
        int x,y;

        switch(c)
        {
        case 'z':
            x=0;y=-1;break;
        case 'q':
             x=-1;y=0;break;
        case 's':
             x=0;y=1;break;
        case 'd':
             x=1;y=0;break;
        default:
            x=y=0;
        }

        for(Entity * e : m_entities)
        {
            NodeComponent& n = (NodeComponent&)e->getComponent(NODE_COMPONENT);
            n.x+=x;
            n.y+=y;
        }
    }
};

class Engine
{
public:
    void addEntity(Entity & e){entities.push_back(e);}
    std::vector<Entity> & getEntities(){return entities;}

    bool update()
    {
        for(System s : m_systems)
        {
            s.update();
        }
        return false;
    }

    void render()
    {
        graphics.update();
    }

private:
    std::vector<Entity> entities;
    std::vector<System> m_systems;
    GraphicSystem graphics;
};

class RessourceMgr
{
public:
    static Entity loadEntity(Entity::Type t)
    {
        Entity e(t);

        switch(t)
        {
        case Entity::Player:

            break;
        case Entity::World:
            break;
        default:;
        }

        return e;
    }
};

void init(Engine & e)
{
    Entity t = RessourceMgr::loadEntity(Entity::World);
    e.addEntity(t);
    t = RessourceMgr::loadEntity(Entity::Player);
    e.addEntity(t);
}


void run(Engine & e)
{
    bool running = true;
    while(running)
    {
        running = e.update();
        e.render();
    }
}

int main()
{
    //Engine e;
    //init(e);
    //run(e);

    GraphicSystem g;

    Entity p(Entity::Player);
    p.addComponent(*new GraphicComponent);
    ((GraphicComponent&)p.getComponent(GRAPHIC_COMPONENT)).c = 'p';
    p.addComponent(*new NodeComponent(5, 3));

    g.addEntity(p);

    //g.update();

    PlayingSystem pl;
    pl.addEntity(p);

    Entity p2(Entity::Player);
    p2.addComponent(*new GraphicComponent('k'));
    p2.addComponent(*new NodeComponent(9, 5));

    g.addEntity(p2);
    pl.addEntity(p2);

    ///MANAGE INCORRECT STATE: PlayingSystem creates incorect states -> either it must not or graphic should not be able to crash cause of an incorect state or we create
    ///     A Sytem dedicated to correcting incorrect states

    ///MESSAGES: here: collision system creates a running=false information

    //bool running = true;
    for(int i = 0; i < 15; i++)
    {
        pl.update();
        g.update();
    }

    return 0;
}
