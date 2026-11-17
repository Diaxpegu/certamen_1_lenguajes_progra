subrutina escribir_unos(n) {
    repetir (n) veces {
        actual, 0 -> siguiente, 1, DER;
        actual, 1 -> siguiente, 1, DER;
        actual, _ -> siguiente, 1, DER;
    }
}

maquina GeneradorUnario {
    alfabeto { 0, 1, _ }
    estados { q_inicio, q_final }
    inicial: q_inicio;
    finales: { q_final };

    transiciones {
        q_inicio -> escribir_unos(5) -> q_final;
    }
}