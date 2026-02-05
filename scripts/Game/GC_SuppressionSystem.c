class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression passing by", params: "0 inf")]
	protected float m_fMaxRange;
	
	[Attribute("10", UIWidgets.Auto, "Min distance range in meters for a bullet to apply suppression from origin", params: "0 inf")]
	protected float m_fMinRange;
	
	[Attribute("1", UIWidgets.Auto, "Distance range in meters for a bullet to apply flinch", params: "0 inf")]
	protected float m_fFlinchRange;
	
	[Attribute("1", UIWidgets.Auto, "Magnitude of screenshake when flinched. 0 = off", params: "0 inf")]
	protected float m_fFlinchShakeMultiplier;
	
	[Attribute("0.5", UIWidgets.Auto, "Multiplier of screenshake when based off suppresion buildup", params: "0 inf")]
	protected float m_fFlinchShakeSuppressedMultiplier;

	[Attribute("5.0", UIWidgets.Auto, "Distance up to which cover is recognized (meters)", params: "0 inf")]
	protected float m_fCoverTraceLength;
	
	[Attribute("7.5", UIWidgets.Auto, "Seconds of no new suppression before recovery starts", params: "0 inf")]
	protected float m_fRecoveryDelay;
	
	[Attribute("0.2", UIWidgets.Auto, "Suppression recovery speed per second", params: "0 1")]
	protected float m_fRecoveryRate;

	[Attribute("2.0", UIWidgets.Auto, "Multiplier applied to projectile mass for suppression", params: "0 inf")]
	protected float m_fMassMultiplier;
	
	[Attribute("0.001", UIWidgets.Auto, "Multiplier applied to projectile speed for suppression", params: "0 inf")]
	protected float m_fSpeedMultiplier;
	
	[Attribute("0.04", UIWidgets.Auto, "Global suppression multiplier per bullet", params: "0 inf")]
	protected float m_fBaseSuppressionMultiplier;
	
	[Attribute("1", UIWidgets.Auto, "Suppression multiplier per bullet hitting near the player", params: "0 inf")]
	protected float m_fHitSuppressionMultiplier;
	
	[Attribute("0.5", UIWidgets.Auto, "Suppression multiplier when target is in cover (0.5 = -50%)", params: "0 inf")]
	protected float m_fCoverMultiplier;

	//! Projectiles tracked by the system
	protected ref array<GC_ProjectileComponent> m_aProjectiles = {};
	
	//! Player suppression value 0-1
	protected float m_fSuppression = 0;
	
	protected int m_iLastUpdateMs = 0;
	protected int m_iLastSuppressionMs = 0;
	
	protected bool m_bIsEnabled = true;
	
	static GC_SuppressionSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;
		return GC_SuppressionSystem.Cast(world.FindSystem(GC_SuppressionSystem));
	}

	override void OnInit()
	{
		super.OnInit();
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return;

		pc.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}
	
	void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		SetEnabled(true);
	}

	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo
			.SetAbstract(false)
			.SetUnique(true)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.FixedFrame);
	}

	override protected void OnUpdate(WorldSystemPoint point)
	{
		super.OnUpdate(point);
		
		if (m_aProjectiles.IsEmpty() && m_fSuppression == 0)
			return;
		
		UpdateProjectiles();
		UpdateSuppression();
	}

	protected void UpdateProjectiles()
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		
		SCR_ChimeraCharacter cc = SCR_ChimeraCharacter.Cast(player);
		vector playerEyePos = cc.EyePosition();
		
		for (int i = m_aProjectiles.Count() - 1; i >= 0; i--)
		{
			GC_ProjectileComponent projectile = m_aProjectiles[i];
			ProjectileMoveComponent move = ProjectileMoveComponent.Cast(projectile.GetOwner().FindComponent(ProjectileMoveComponent));
			if(!move)
				return;
			
		    vector projPos = projectile.GetOwner().GetOrigin();
		
		    // Check if projectile is moving toward the player
		    float approach = vector.Dot(playerEyePos - projPos, move.GetVelocity());
			
		    if (approach <= 0)
		    {
				// Projectile is no longer approaching → check distance
				vector flybyPos = GetFlybyPoint(projPos, projectile.position, playerEyePos);
				float dist = vector.Distance(flybyPos, playerEyePos);
				
				if (dist <= m_fMaxRange)
				{
					float multi = 1;
					if(IsInCover(projectile))
						multi -= m_fCoverMultiplier;
					
					float suppression = GetBulletSuppression(projectile, dist, multi);
					AddSuppression(suppression);
				}

				if (dist <= m_fFlinchRange)
					Flinch();

		        m_aProjectiles.Remove(i);
		        continue;
		    }

			projectile.position = projPos;
		}
	}
	
	protected void AddSuppression(float suppression)
	{
		if (suppression <= 0)
			return;
		
		m_fSuppression = Math.Clamp(m_fSuppression + suppression, 0, 1);
		
		m_iLastSuppressionMs = System.GetTickCount();
		
		UpdateEffect();
	}
	
	protected float GetBulletSuppression(GC_ProjectileComponent projectile, float distance, float multiplier = 1)
	{
		IEntity projEntity = projectile.GetOwner();
		if(!projEntity)
			return 0;
		
		ProjectileMoveComponent move = ProjectileMoveComponent.Cast(projEntity.FindComponent(ProjectileMoveComponent));
		if(!move)
			return 0;

		BaseContainer container = move.GetComponentSource(projEntity);
		if(!container)
			return 0;
		
		float mass;
		container.Get("Mass", mass);
		if (!mass)
			return 0;
		
		float speed = move.GetVelocity().Length();
		
		float distanceTerm = 0.0;
		if (distance <= 0.0)
			distanceTerm = 1.0;
		else if (distance < m_fMaxRange)
			distanceTerm = 1.0 - (distance / m_fMaxRange);

		float suppression = (mass * m_fMassMultiplier);
		suppression += (speed * m_fSpeedMultiplier);
		suppression *= distanceTerm;
	
		suppression = suppression * m_fBaseSuppressionMultiplier * multiplier;
		
		suppression = Math.Clamp(suppression, 0, 1);
		
		return suppression;
	}
	
	protected float GetBulletSuppression(IEntity bullet, float distance, float multiplier = 1)
	{
		ProjectileMoveComponent move = ProjectileMoveComponent.Cast(bullet.FindComponent(ProjectileMoveComponent));
		if (!move)
			return 0;
		
		BaseContainer container = move.GetComponentSource(bullet);
		if(!container)
			return 0;
		
		float mass;
		container.Get("Mass", mass);
		if (!mass)
			return 0;
		
		float speed = move.GetVelocity().Length();
		
		float distanceTerm = 0.0;
		if (distance <= 0.0)
			distanceTerm = 1.0;
		else if (distance < m_fMaxRange)
			distanceTerm = 1.0 - (distance / m_fMaxRange);

		float suppression = (mass * m_fMassMultiplier);
		suppression += (speed * m_fSpeedMultiplier);
		suppression *= distanceTerm;
	
		suppression = suppression * m_fBaseSuppressionMultiplier * multiplier;
		
		suppression = Math.Clamp(suppression, 0, 1);
		
		return suppression;
	}
	
	protected void UpdateSuppression()
	{
		int now = System.GetTickCount();
	
		float deltaSec = (now - m_iLastUpdateMs) / 1000.0;
		m_iLastUpdateMs = now;
	
		if (m_fSuppression <= 0)
			return;
	
		float sinceSuppSec = (now - m_iLastSuppressionMs) / 1000.0;
	
		if (sinceSuppSec < m_fRecoveryDelay)
			return;
	
		float decay = m_fRecoveryRate * deltaSec;

		m_fSuppression = Math.Clamp(m_fSuppression - decay, 0, 1);
		
		UpdateEffect();
	}
	
	protected void UpdateEffect()
	{
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if (sse)
			sse.UpdateSuppresion();
	}
	
	protected void Flinch()
	{
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if (sse)
			sse.Flinch();
		
		if (m_fFlinchShakeMultiplier > 0)
		{
			float suppressionMulti = 1.0 + m_fSuppression * m_fFlinchShakeSuppressedMultiplier;
	
			float magnitude = m_fFlinchShakeMultiplier * suppressionMulti;
			
			ScreenShake(magnitude);
		}
	}
	
	protected void ScreenShake(float magnitude = 1, float inTime = 0.01, float sustainTime = 0.1, float outTime = 0.24)
	{
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		SCR_CameraShakeManagerComponent.AddCameraShake(magnitude, magnitude, inTime, sustainTime, outTime);
	}
	
	//(m_eSuppType, source, transform, speed);
	void HandleBulletImpact(IEntity bullet, vector direction, float distance, float speed)
	{
		float multi = m_fHitSuppressionMultiplier;
		if(IsInCover(direction))
			multi -= m_fCoverMultiplier;
		
		float suppression = GetBulletSuppression(bullet, distance, multi);
		AddSuppression(suppression);
		
		//PrintFormat("GC | HandleBulletImpact: %1 - %2 - %3", bullet, distance, speed);
	}
	
	void RegisterProjectile(IEntity projectile)
	{
		if (!m_bIsEnabled)
			return;
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		
		GC_ProjectileComponent projComp = GC_ProjectileComponent.Cast(projectile.FindComponent(GC_ProjectileComponent));
		if (!projComp)
			return;
		
		if (m_aProjectiles.Contains(projComp))
			return;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if (!moveComp)
			return;

		projComp.position = projectile.GetOrigin();
		
		//PrintFormat("GC | RegisterProjectile: %1", projComp);
		m_aProjectiles.Insert(projComp);
	}
	
	void UnregisterProjectile(GC_ProjectileComponent projectile)
	{
		if(m_aProjectiles.Contains(projectile))
			m_aProjectiles.RemoveItem(projectile);
	}
	
	protected bool IsInCover(vector direction)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!player)
			return false;

		vector start = player.EyePosition();
		vector end = start - direction * m_fCoverTraceLength;
		
		
		// Trace from eyes toward where the bullet came from
		TraceParam tp = MakeTraceParam(start, end, TraceFlags.ENTS | TraceFlags.WORLD);
		tp.Exclude = player;
		
		float fraction = GetGame().GetWorld().TraceMove(tp);
		
		//if(fraction < 1.0)
		//	m_shapes.Insert(Shape.Create(ShapeType.LINE, Color.GREEN, ShapeFlags.DEFAULT, start, end));
		//else
		//	m_shapes.Insert(Shape.Create(ShapeType.LINE, Color.RED, ShapeFlags.DEFAULT, start, end));
		
		return fraction < 1.0;
	}
	
	//Player is consider in cover if they can't be hit by a bullet
	protected bool IsInCover(GC_ProjectileComponent projectile)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!player)
			return false;

		vector playerEyePos = player.EyePosition();
		
		if (!projectile)
			return false;
		
		ProjectileMoveComponent move = ProjectileMoveComponent.Cast(projectile.GetOwner().FindComponent(ProjectileMoveComponent));
		if(!move)
			return false;
		
		vector velocity = move.GetVelocity();
		if (velocity.LengthSq() <= 0.0001)
			return false;
		
		// Direction FROM player TOWARD shooter (opposite bullet direction)
		vector dir = -velocity;
		dir.Normalize();
		
		vector start = playerEyePos;
		vector end = start + dir * m_fCoverTraceLength;
		
		// Trace from eyes toward where the bullet came from
		TraceParam tp = MakeTraceParam(start, end, TraceFlags.ENTS | TraceFlags.WORLD);
		tp.Exclude = player;
		
		float fraction = GetGame().GetWorld().TraceMove(tp);
		
		return fraction < 1.0;
	}

	protected vector GetFlybyPoint(vector currPos, vector prevPos, vector playerPos)
	{
		vector seg = currPos - prevPos;
		float segLenSq = vector.DistanceSq(prevPos, currPos);
	
		if (segLenSq <= 0.000001)
			return prevPos;
	
		vector toPlayer = playerPos - prevPos;
	
		float t = vector.Dot(toPlayer, seg) / segLenSq;
		t = Math.Clamp(t, 0.0, 1.0);
	
		vector closestPoint = prevPos + seg * t;
		
		return closestPoint;
	}

	float GetAmount()
	{
		return m_fSuppression;
	}
	
	float GetMaxRange()
	{
		return m_fMaxRange;
	}
	
	float GetMinRange()
	{
		return m_fMinRange;
	}
	
	void SetEnabled(bool isEnabled)
	{

		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		if (sem)
		{
			GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
			if (sse)
			{
				sse.SetEnabled(isEnabled);
				sse.ClearEffects();
			}
		}

		m_fSuppression = 0;
		m_aProjectiles.Clear();
		
		m_bIsEnabled = isEnabled;
	}
	
	//! Debug shapes, remove later
	/*
	protected ref array<ref Shape> m_shapes = {};
	protected void CreateDebugCircle(vector position, int color = Color.RED, bool clear = false)
	{
		if (clear)
			m_shapes.Clear();
		
		m_shapes.Insert(Shape.CreateSphere(color, ShapeFlags.DEFAULT, position, 0.2));
	}
	*/
}
