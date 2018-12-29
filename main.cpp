#include <iostream>
#include <vector>
#include <conio.h>
#include <list>
#include <algorithm>
#include <iterator>
#include <functional>

const int EMPTY_COMPONENT = 0;
const int GRAPHIC_COMPONENT = 1;
const int NODE_COMPONENT = 2;
const int GEOM_COMPONENT = 3;
const int COLLISION_COMPONENT = 4;
const int SCRIPT_COMPONENT = 5;

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
        oldX=x;
        oldY=y;
    }

    int x, y;
    int oldX,oldY;
};

bool operator<(const NodeComponent& e1, const NodeComponent& e2)
{
    return e1.x < e2.x || (e1.x == e2.x && e1.y < e2.y);
}

class EmptyComponent : public Component
{
private:
    EmptyComponent() : Component(EMPTY_COMPONENT)
    {

    }
    static EmptyComponent * me;
public:
    static EmptyComponent &getInstance()
    {
        if(me == nullptr)
        {
            me = new EmptyComponent;
        }

        return *me;
    }
};

EmptyComponent * EmptyComponent::me = nullptr;

class GeomComponent : public Component
{
public:
    GeomComponent() : Component(GEOM_COMPONENT){}
    GeomComponent(std::vector<NodeComponent>  points) : GeomComponent()
    {
        m_points = points;
    }

    std::vector<NodeComponent>& getPoints()
    {
        return m_points;
    }
private:
    std::vector<NodeComponent> m_points;
};

///TODO:Find a way to use only one type of script for both collsion reaction and scripts
class CollisionComponent : public Component
{
public:
    CollisionComponent(std::function<void()> s) : Component(COLLISION_COMPONENT), script(s)
    {

    }
    void resolve()
    {
        script();
    }
private:
    std::function<void()> script;
};

class ScriptComponent : public Component
{
public:
    ScriptComponent(std::function<void()> s) : Component(SCRIPT_COMPONENT), script(s)
    {

    }
    void execute()
    {
        script();
    }
private:
    std::function<void()> script;
};

////////////////////////////////////////////////

class Entity
{
public:
    enum Type{Player, PNJ, Ex};
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

    Component& getComponent(int id)
    {
        for(Component * c: m_components)
        {
            if(c->getType() == id)
                return *c;
        }

        return EmptyComponent::getInstance();
    }

    bool hasComponent(int id)
    {
        return !dynamic_cast<EmptyComponent*>(&getComponent(id));
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

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

class PlayingSystem : public System
{
public:
    void update()
    {
        char c = getch();
        int x,y;

        switch(c)
        {
        case 'z'://case KEY_UP:
            x=0;y=-1;break;
        case 'q'://case KEY_LEFT:
             x=-1;y=0;break;
        case 's'://case KEY_DOWN:
             x=0;y=1;break;
        case 'd'://case KEY_RIGHT:
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

enum {EXIT_REACHED, COLLISION, NPC1_COLLISION_INTERACT};

class MessageSystem : public System
{
public:
    static void sendMessage(int id)
    {
        msg.insert(msg.end(), id);
    }

    static std::list<int>& getMessages()
    {
        return msg;
    }

private:
    static std::list<int> msg;
};

std::list<int> MessageSystem::msg;

///Only collisions here (no velocity update, ...)
class PhysicSystem : public System
{
public:
    std::vector<Entity*>& getMovingEntities()
    {
        //Returns only moving entities (todo)
        return m_entities;
    }

    std::vector<Entity*>& getFixedEntities(Entity * e)
    {
        return m_entities;
    }

    void update()
    {
        ///Detect and Resolve collisions
        ///Collisions between entities and borders:(10x44)


        ///I suppose that I have for now only one "batch"
        auto currEntity = getMovingEntities().begin();
        for(Entity * e : getMovingEntities())
        {
            ///Collisions with fixed entities
            NodeComponent & n = (NodeComponent&)e->getComponent(NODE_COMPONENT);

            if(n.x < 0)
            {
                n.x=0;
            }
            else if(n.x >= 44)
            {
                n.x=43;
            }

            if(n.y < 0)
            {
                n.y=0;
            }
            else if(n.y >= 10)
            {
                n.y=9;
            }

            currEntity++;

            for(auto it = currEntity; it != getMovingEntities().end(); it++)
            {
                doCollisions(e, *it);
            }
        }
    }

    std::vector<NodeComponent> getPoints(Entity * e)
    {
        ///Different types of shapes(components)

        std::vector<NodeComponent> points;
        NodeComponent& n2 = ((NodeComponent&)e->getComponent(NODE_COMPONENT));

        if(e->hasComponent(GEOM_COMPONENT))
        {
            for(NodeComponent &n : ((GeomComponent&)e->getComponent(GEOM_COMPONENT)).getPoints())
            {
                points.push_back(NodeComponent(n2.x+n.x, n2.y+n.y));
            }
        }
        points.push_back(n2);

        return points;
    }

    void doCollisions(Entity * e1, Entity * e2)
    {
        std::vector<NodeComponent> points1 = getPoints(e1);
        std::vector<NodeComponent> points2 = getPoints(e2);

        std::sort(points1.begin(), points1.end());
        std::sort(points2.begin(), points2.end());

        std::vector<NodeComponent> inter;

        std::set_intersection(points1.begin(), points1.end(), points2.begin(), points2.end(), std::back_inserter(inter));

        if(!inter.empty())
        {
            if(e1->hasComponent(COLLISION_COMPONENT))
            {
                ((CollisionComponent&)e1->getComponent(COLLISION_COMPONENT)).resolve();
            }

            if(e2->hasComponent(COLLISION_COMPONENT))
            {
                ((CollisionComponent&)e2->getComponent(COLLISION_COMPONENT)).resolve();
            }
        }
    }
};

class GUISystem : public System
{
public:
    void update()
    {
        /*auto it = MessageSystem::getMessages().begin();

        for(int m : MessageSystem::getMessages())
        {
            if(m == COLLISION)
            {
                std::cout << "Collision !" ;//<< std::endl;
                MessageSystem::getMessages().erase(it);

            }
            it++;
        }*/

        std::list<int> & msg = MessageSystem::getMessages();
        auto itEnd = msg.end();
        auto it = msg.begin();

        while(it != itEnd)
        {
            bool delMsg = true;
            switch(*it)
            {
            case COLLISION:
                std::cout << "Collision !";
                break;
            case NPC1_COLLISION_INTERACT:
                std::cout << "Hello my king !";
                break;
            default:
                delMsg = false;
            }
            if(delMsg)
            {
                auto it2 = it;
                it++;
                msg.erase(it2);
            }
            else
            {
                it++;
            }
        }
    }
};

class ScriptSystem : public System
{
public:
    void update()
    {
        for(Entity * e : m_entities)
        {
            ((ScriptComponent&)e->getComponent(SCRIPT_COMPONENT)).execute();
        }
    }
};

bool isRunning()
{
    for(int m : MessageSystem::getMessages())
    {
        if(m == EXIT_REACHED)
            return false;
    }
    return true;
}

int main()
{
    GraphicSystem g;
    PlayingSystem pl;
    PhysicSystem phys;
    GUISystem gui;
    ScriptSystem scriptEngine;

    Entity p(Entity::Player);
    p.addComponent(*new GraphicComponent);
    ((GraphicComponent&)p.getComponent(GRAPHIC_COMPONENT)).c = 'p';
    p.addComponent(*new NodeComponent(5, 3));

    g.addEntity(p);
    pl.addEntity(p);
    phys.addEntity(p);

    Entity p2(Entity::PNJ);
    p2.addComponent(*new GraphicComponent('k'));
    p2.addComponent(*new NodeComponent(9, 5));
    p2.addComponent(*new GeomComponent({{-1, 0}, {1, 0}, {0, 1}, {0, -1}}));
    p2.addComponent(*new CollisionComponent([]()
                                            {
                                                ///Two options: create an entity and a system that manages entity life(we give like 1 step of living and we increment it in each collsion
                                                                        ///so that it keeps constant as long as we are colliding)
                                                /// OR: we create an entity that has a script that listens to those collsions so that we can react when there is no longer a collision
                                                ///(Those options are basically the same except one is an entity, the other a system)

                                                ///Or I could just run a script in this entity (with a scriptSystem) and read messages that I would send myself ... yeah
                                                MessageSystem::sendMessage(NPC1_COLLISION_INTERACT);
                                            }));
    p2.addComponent(*new ScriptComponent([]()
                                        {
                                            static bool wasInColl = false;
                                            std::list<int> & msg = MessageSystem::getMessages();
                                            auto itEnd = msg.end();
                                            auto it = msg.begin();

                                            bool coll = false;

                                            while(it != itEnd)
                                            {
                                                if(*it == NPC1_COLLISION_INTERACT)
                                                {
                                                    coll = true;
                                                    auto it2 = it;
                                                    it++;
                                                    msg.erase(it2);
                                                    //MessageSystem::sendMessage(NPC1_COLLISION_INTERACT);
                                                }
                                                else
                                                {
                                                    it++;
                                                }
                                            }

                                            if(coll)
                                            {
                                                //allow for behavior
                                                MessageSystem::sendMessage(NPC1_COLLISION_INTERACT);
                                            }
                                            else if(wasInColl)
                                            {

                                            }

                                        }));

    g.addEntity(p2);
    phys.addEntity(p2);
    scriptEngine.addEntity(p2);

    Entity ex(Entity::Ex);
    ex.addComponent(*new NodeComponent(43, 9));
    ex.addComponent(*new CollisionComponent([](){MessageSystem::sendMessage(EXIT_REACHED);}));

    phys.addEntity(ex);

    bool running = true;
    while(running)
    {
        g.update();
        gui.update();
        pl.update();
        phys.update();
        scriptEngine.update();

        running = isRunning();
    }

    return 0;
}
