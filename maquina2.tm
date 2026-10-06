// Subrutina para escribir 'n' unos en la cinta
subrutina escribir_unos(n) { }

maquina GeneradorUnario {
    alfabeto { 0, 1, _ }
    estados { q_inicio, q_final }
    inicial: q_inicio;
    finales: { q_final };

    // El parser registra en RAM los estados fantasma: sub_escribir_unos_00 hasta 04
    usa escribir_unos(5);

    transiciones {
        // 1. El estado inicial pasa el control al primer estado de la subrutina
        q_inicio, _ -> sub_escribir_unos_00, 1, DER;

        // 2. Transiciones de la subrutina (Generan 5 unos y avanzan)
        sub_escribir_unos_00, _ -> sub_escribir_unos_01, 1, DER;
        sub_escribir_unos_01, _ -> sub_escribir_unos_02, 1, DER;
        sub_escribir_unos_02, _ -> sub_escribir_unos_03, 1, DER;
        sub_escribir_unos_03, _ -> sub_escribir_unos_04, 1, DER;

        // 3. El último paso de la subrutina se detiene y entrega el control al estado final
        sub_escribir_unos_04, _ -> q_final, 1, QUIETO;
    }
}
