#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "PhysBody3D.h"
#include "ModuleInput.h"
#include "Primitive.h"

#ifdef _DEBUG
	#pragma comment (lib, "Bullet/libx86/BulletDynamics_debug.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision_debug.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath_debug.lib")
#else
	#pragma comment (lib, "Bullet/libx86/BulletDynamics.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath.lib")
#endif

ModulePhysics3D::ModulePhysics3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	m_debugDisplay = false;

	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	debug_draw = new DebugDrawer();
}

// Destructor
ModulePhysics3D::~ModulePhysics3D()
{
	delete debug_draw;
	delete solver;
	delete broad_phase;
	delete dispatcher;
	delete collision_conf;
}

// Render not available yet----------------------------------
bool ModulePhysics3D::Init()
{
	LOG("Creating 3D Physics simulation");
	bool ret = true;

	return ret;
}

// ---------------------------------------------------------
void ModulePhysics3D::Start()
{
	LOG("Creating Physics environment");

	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	world->setDebugDrawer(debug_draw);
	world->setGravity(GRAVITY);
}

// ---------------------------------------------------------
UpdateStatus ModulePhysics3D::PreUpdate()
{
//	world->stepSimulation(dt, 15);

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		if(numContacts > 0)
		{
			PhysBody3D* pbodyA = (PhysBody3D*)obA->getUserPointer();
			PhysBody3D* pbodyB = (PhysBody3D*)obB->getUserPointer();

			if(pbodyA && pbodyB)
			{
				std::list<Module*>::iterator item = pbodyA->m_collisionListeners.begin();
				while(item != pbodyA->m_collisionListeners.end())
				{
					(*item)->OnCollision(pbodyA, pbodyB);
					item++;
				}

				item = pbodyB->m_collisionListeners.begin();
				while(item != pbodyB->m_collisionListeners.end())
				{
					(*item)->OnCollision(pbodyB, pbodyA);
					item++;
				}
			}
		}
	}
	return UpdateStatus::Continue;
}

// ---------------------------------------------------------
UpdateStatus ModulePhysics3D::Update()
{
	if(App->m_input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		m_debugDisplay = !m_debugDisplay;

	if(m_debugDisplay == true)
	{
		world->debugDrawWorld();
	}

	return UpdateStatus::Continue;
}

// Called before quitting 
void ModulePhysics3D::CleanUp()
{
	LOG("Destroying 3D Physics simulation");
	//world->removeRigidBody(ground);
	//delete ground;
	m_ground = nullptr;

	// Remove from the world all collision bodies
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	for(std::list<btTypedConstraint*>::iterator item = m_constraints.begin(); item != m_constraints.end(); item++)
	{
		world->removeConstraint(*item);
		delete (*item);
	}
	
	m_constraints.clear();

	for(std::list<btDefaultMotionState*>::iterator item = m_motions.begin(); item != m_motions.end(); item++)
		delete (*item);

	m_motions.clear();

	for(std::list<btCollisionShape*>::iterator item = m_shapes.begin(); item != m_shapes.end(); item++)
		delete (*item);

	m_shapes.clear();

	for(std::list<PhysBody3D*>::iterator item = m_bodies.begin(); item != m_bodies.end(); item++)
		delete (*item);

	m_bodies.clear();

	delete world;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const P_Sphere& sphere, float mass)
{
	btCollisionShape* colShape = new btSphereShape(sphere.m_radius);
	m_shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(sphere.m_transform.ptr());

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	m_motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	world->addRigidBody(body);
	m_bodies.push_back(pbody);

	return pbody;
}


// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const P_Cube& cube, float mass)
{
	btCollisionShape* colShape = new btBoxShape(btVector3(cube.m_size.x*0.5f, cube.m_size.y*0.5f, cube.m_size.z*0.5f));
	m_shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(cube.m_transform.ptr());

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	m_motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	world->addRigidBody(body);
	m_bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const P_Cylinder& cylinder, float mass)
{
	btCollisionShape* colShape = new btCylinderShapeX(btVector3(cylinder.m_height*0.5f, cylinder.m_radius, 0.0f));
	m_shapes.push_back(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(cylinder.m_transform.ptr());

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	m_motions.push_back(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	world->addRigidBody(body);
	m_bodies.push_back(pbody);

	return pbody;
}

// ---------------------------------------------------------
void ModulePhysics3D::AddConstraintP2P(PhysBody3D& bodyA, PhysBody3D& bodyB, const math::float3& anchorA, const math::float3& anchorB)
{
	btTypedConstraint* p2p = new btPoint2PointConstraint(
		*(bodyA.m_body), 
		*(bodyB.m_body), 
		btVector3(anchorA.x, anchorA.y, anchorA.z), 
		btVector3(anchorB.x, anchorB.y, anchorB.z));
	world->addConstraint(p2p);
	m_constraints.push_back(p2p);
	p2p->setDbgDrawSize(2.0f);
}

void ModulePhysics3D::AddConstraintHinge(PhysBody3D& bodyA, PhysBody3D& bodyB, const math::float3& anchorA, const math::float3& anchorB, const math::float3& axisA, const math::float3& axisB, bool disable_collision)
{
	btHingeConstraint* hinge = new btHingeConstraint(
		*(bodyA.m_body), 
		*(bodyB.m_body), 
		btVector3(anchorA.x, anchorA.y, anchorA.z),
		btVector3(anchorB.x, anchorB.y, anchorB.z),
		btVector3(axisA.x, axisA.y, axisA.z), 
		btVector3(axisB.x, axisB.y, axisB.z));

	world->addConstraint(hinge, disable_collision);
	m_constraints.push_back(hinge);
	hinge->setDbgDrawSize(2.0f);
}

void ModulePhysics3D::DeleteBody(PhysBody3D* body)
{
	m_bodies.remove(body);
	//Warning, untested
	//bodies.del(bodies.findNode(body));
	world->removeRigidBody(body->m_body);
	delete body;
	body = nullptr;
}

// =============================================
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	m_line.m_a.Set(from.getX(), from.getY(), from.getZ());
	m_line.m_b.Set(to.getX(), to.getY(), to.getZ());
	m_line.m_color.Set(color.getX(), color.getY(), color.getZ());
	m_line.Render();
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	m_primitivePoint.m_transform.Translate(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	m_primitivePoint.m_color.Set(color.getX(), color.getY(), color.getZ());
	m_primitivePoint.Render();
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	LOG("Bullet warning: %s", warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	LOG("Bullet draw text: %s", textString);
}

void DebugDrawer::setDebugMode(int debugMode)
{
	m_drawMode = (DebugDrawModes) debugMode;
}

int	 DebugDrawer::getDebugMode() const
{
	return m_drawMode;
}
