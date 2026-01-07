class GC_SupressionControllerClass : ScriptComponentClass
{
}

class GC_SupressionController : ScriptComponent
{
	static protected GC_SupressionController m_instance;
	
	protected ref array<ref GC_Projectile> m_projectiles = {};
	
	protected float m_Suppression = 0;
	
	protected void UpdateProjectiles()
	{
		foreach(GC_Projectile projectile : m_projectiles)
		{
			
			lastPosition = projectile.bullet.GetOrigin();
		}
	}
	
	static void Register(IEntity effect, BaseMuzzleComponent muzzle, IEntity projectile)
	{
		Print("GC | Projectile Registered: " + projectile);
		
		GC_Projectile proj = new GC_Projectile();
		proj.effect = effect;
		proj.muzzle = muzzle;
		proj.bullet = projectile;

		ShellMoveComponent shellComp = ShellMoveComponent.Cast(projectile.FindComponent(ShellMoveComponent));
		proj.shell = shellComp.GetComponentSource(projectile);
		
		if(m_instance)
			m_instance.m_projectiles.Insert(proj);
	}
	
	override protected void OnPostInit(IEntity owner)
	{
		m_instance = this;
	}
	
	static GC_SupressionController GetInstance()
	{
		return m_instance;
	}
}

class GC_Projectile
{
	IEntity effect;
	BaseMuzzleComponent muzzle;
	IEntity bullet;
	BaseContainer shell;
	vector lastPosition;
}