maquina IncrementadorBinario {
    alfabeto { 0, 1, _ }
    estados { q_buscar_fin, q_sumar, q_terminar }
    inicial: q_buscar_fin;
    finales: { q_terminar };

    transiciones {
        // Bucle para avanzar hasta el final de la cadena binaria
        q_buscar_fin, 0 -> q_buscar_fin, 0, DER;
        q_buscar_fin, 1 -> q_buscar_fin, 1, DER;

        // Al encontrar el blanco, retrocede para empezar a sumar (simula el fin del bucle de lectura)
        q_buscar_fin, _ -> q_sumar, _, IZQ;

        // Propagación de acarreo (si lee 1, lo convierte en 0 y sigue retrocediendo)
        q_sumar, 1 -> q_sumar, 0, IZQ;

        // Si encuentra un 0 o un espacio al retroceder, lo convierte en 1 y termina
        q_sumar, 0 -> q_terminar, 1, QUIETO;
        q_sumar, _ -> q_terminar, 1, QUIETO;
    }
}
