maquina Complemento {
    alfabeto { 0, 1, _ }
    estados { q0, q_final }
    inicial: q0;
    finales: { q_final };

    transiciones {
        // Si lee 0, escribe 1 y avanza
        q0, 0 -> q0, 1, DER;
        
        // Si lee 1, escribe 0 y avanza
        q0, 1 -> q0, 0, DER;
        
        // Si lee blanco, termina
        q0, _ -> q_final, _, QUIETO;
    }
}
