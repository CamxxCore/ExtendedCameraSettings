#pragma once

static LPCTSTR en_us[]{
	"First Person Field of View",
	"First Person Always Use Reticle",
	"First Person Min Pitch Angle",
	"First Person Max Pitch Angle",
	"Third Person Field of View",
	"Third Person Horizontal Origin",
	"Third Person Vertical Origin",
	"Third Person Follow Distance",
	"Third Person Running Shake",
	"Third Person Min Pitch Angle",
	"Third Person Max Pitch Angle",
	"Third Person Aiming Field of View",
	"Third Person Aiming Horizontal Origin",
	"Third Person Aiming Follow Distance",
	"First Person Vehicle Field of View",
	"First Person Vehicle Vertical Origin",
	"First Person Vehicle Switch In Water",
	"First Person Vehicle Switch on Destroyed",
	"First Person Vehicle Min Pitch Angle",
	"First Person Vehicle Max Pitch Angle",
	"Third Person Vehicle Field of View",
	"Third Person Vehicle Min Pitch Angle",
	"Third Person Vehicle Max Pitch Angle",
	"Third Person Vehicle Auto Center",
	"Third Person Vehicle Follow Distance",
	"Third Person Vehicle Pivot Scale",
	"Third Person Vehicle Horizontal Origin",
	"Third Person Vehicle High Speed Shake"
};

 static LPCTSTR en_es[] {
	 u8"Campo de visión en Primera Persona",
	 u8"Usar siempre el retículo en Primera Persona",
	 u8"Ángulo de Inclinación Mínimo en Primera Persona",
	 u8"Ángulo de Inclinación Máximo en Primera Persona",
	 u8"Campo de Visión en Tercera Persona",
	 u8"Origen Horizontal en Tercera Persona",
	 u8"Origen Vertical en Tercera Persona",
	 u8"Distancia de Seguimiento en Tercera Persona",
	 u8"Agitación Corriendo en Tercera Persona",
	 u8"Ángulo de Inclinación Mínimo en Tercera Persona",
	 u8"Ángulo de Inclinación Máximo en Tercera Persona",
	 u8"Campo de Visión Apuntando en Tercera Persona",
	 u8"Origen Horizontal Apuntando en Tercera Persona",
	 u8"Distancia de Seguimiento Apuntando en Tercera Persona",
	 u8"Campo de Visión de Vehículo en Primera Persona",
	 u8"Origen Vertical de Vehículo en Primera Persona",
	 u8"Cambiar la Primera Persona de Vehículo en Agua",
	 u8"Cambiar la Primera Persona del Vehículo Destruido",
	 u8"Ángulo de Inclinación Mínimo en Vehículo en Primera Persona",
	 u8"Ángulo de Inclinación Máximo en Vehículo en Primera Persona",
	 u8"Campo de Visión en Vehículo en Tercera Persona",
	 u8"Ángulo de Inclinación Mínimo en Vehículo en Tercera Persona",
	 u8"Ángulo de Inclinación Máximo en Vehículo en Tercera Persona",
	 u8"Auto - centrado en Vehículo en Tercera Persona",
	 u8"Distancia de seguimiento de Vehículo en Tercera Persona",
	 u8"Escala del Pivote del Vehículo en Tercera Persona",
	 u8"Origen Horizontal en Vehículo en Tercera Persona",
	 u8"Agitación de Alta Velocidad en Vehículo en Tercera Persona"
 };

 static LPCTSTR* langtext_array[2]{ 
	 en_us, 
	 en_es 
 };

 inline LPCTSTR getConstString(int langugageId, GlobalTextEntry_t textId)
 {
	 return langtext_array[langugageId][textId];
 }