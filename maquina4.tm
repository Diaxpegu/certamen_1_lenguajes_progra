subrutina escribir_unos(n) {
    repetir (n) veces {
        actual, 0 -> siguiente, 1, DER;
        actual, 1 -> siguiente, 1, DER;
        actual, _ -> siguiente, 1, DER;
    }
}
subrutina escribir_ceros(n){
    repetir(n) veces {
        actual, 0 -> siguiente, 0, IZQ;
        actual, 1 -> siguiente, 0, IZQ;
        actual, _ -> siguiente, 0, IZQ;

    }

}

maquina GeneradorUnario {
    alfabeto { 0, 1, _ }
    estados { q_inicio, q_final, q_intermedio }
    inicial: q_inicio;
    finales: { q_final };

    transiciones {
        q_inicio -> escribir_unos(5) -> q_intermedio; //Escribe y despues se mueve
    
        q_intermedio -> escribir_ceros(6) -> q_final;
    }
}