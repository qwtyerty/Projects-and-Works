using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class QuizControl3 : MonoBehaviour
{
    private QuestionCollector3 questionCollection;
    private Questions currentQuestion;
    private UIControl uiController;


    [SerializeField]
    private float delayBetweenQuestions = 1f;

    public bool isCorrect = false;

    
    public Image b1;
    public Image b2;
    public Image b3;
    public Image b4;

    public GameObject quizPanel;
    public GameObject nextRoom;


    private void Awake()
    {
        questionCollection = FindObjectOfType<QuestionCollector3>();
        uiController = FindObjectOfType<UIControl>();
    }

    private void Start()
    {
        showQuestion();
    }

    public void update() { }

    // Get the question and answer and set up the UI
    private void showQuestion()
    {
        currentQuestion = questionCollection.GetUnaskedQuestion();
        if (currentQuestion != null)
        {
            b1.color = Color.white;
            b2.color = Color.white;
            b3.color = Color.white;
            b4.color = Color.white;
            uiController.SetupUI(currentQuestion);
        }
        else
        {
            quizPanel.SetActive(false);
            nextRoom.SetActive(true);

        }
    }

    // Submits and Answer and highlights and continues or highlights red and promp hint
    public void SubmitAnswer(int answerNumber)
    {
        Debug.Log(answerNumber);
        Debug.Log(currentQuestion.correct);
        bool isCorrect = answerNumber == currentQuestion.correct;

        uiController.SubmittedAnswer(isCorrect);
        if (isCorrect)
        {
            // Mark the button used as the answer green then bring up the next question after a delay
            switch (answerNumber)
            {
                case 0:
                    b1.color = Color.green;
                    Debug.Log(b1.color);
                    break;
                case 1:
                    b2.color = Color.green;
                    Debug.Log(b2.color);
                    break;
                case 2:
                    b3.color = Color.green;
                    Debug.Log(b3.color);
                    break;
                case 3:
                    b4.color = Color.green;
                    Debug.Log(b4.color);
                    break;
            }
            StartCoroutine(ShowNextQuestionAfterDelay());
        }
        else
        {
            // Mark the answer red
            switch (answerNumber)
            {
                case 0:
                    b1.color = Color.red;
                    Debug.Log(b1.color);
                    break;
                case 1:
                    b2.color = Color.red;
                    Debug.Log(b2.color);
                    break;
                case 2:
                    b3.color = Color.red;
                    Debug.Log(b3.color);
                    break;
                case 3:
                    b4.color = Color.red;
                    Debug.Log(b4.color);
                    break;
            }
        }

    }

    private IEnumerator ShowNextQuestionAfterDelay()
    {
        yield return new WaitForSeconds(delayBetweenQuestions);
        showQuestion();
    }
}
